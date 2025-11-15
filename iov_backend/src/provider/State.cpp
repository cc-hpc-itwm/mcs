// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cassert>
#include <chrono>
#include <fmt/format.h>
#include <functional>
#include <iterator>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/iov_backend/Locations.hpp>
#include <mcs/iov_backend/provider/State.hpp>
#include <mcs/iov_backend/provider/distribution/AsEqualAsPossible.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/serialization/STD/unordered_map.hpp>
#include <mcs/util/divru.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <memory>
#include <stdexcept>

namespace mcs::iov_backend::provider
{
  State::State
    ( storage::ID next_storage_id
    , state::Storages storages
    , state::Collections collections
    )
      : _next_storage_id {next_storage_id}
      , _storages {storages}
      , _collections {collections}
  {}

  auto State::distribute_as_equal_as_possible
    ( util::not_null<StoragesClients> storages_clients
    , core::memory::Size size
    , core::memory::Offset offset
    ) const -> UsedStorages
  {
    auto const capacities
      { std::invoke
        ( [&]
          {
            auto available
              { core::storage::MaxSize
                { core::storage::MaxSize::Limit {core::memory::make_size (0)}
                }
              };

            auto _capacities
              { std::priority_queue
                  < distribution::AsEqualAsPossible::StorageIDAndCapacity
                  >{}
              };

            // \todo for exclusive storage the capacity is a local
            // information
            storages_clients->capacities
              ( _storages
              , [&] (auto storage_id, auto capacity)
                {
                  available = available + capacity;

                  _capacities.emplace (storage_id, capacity);
                }
              );

            if (size > available)
            {
              // \todo specific exception
              throw mcs::Error
                { fmt::format
                  ( "distribute_as_equal_as_possible: "
                    "size '{}' > available '{}'"
                  , size
                  , available
                  )
                };
            }

            return _capacities;
          }
        )
      };

    // \todo better distribution across (some of) the storages
    return storages_clients->create
      ( _storages
      , [ ranges {distribution::AsEqualAsPossible{} (size, capacities)}
        , offset
        ] (auto storage_id, auto const&)
        {
          return core::memory::shift (ranges.at (storage_id), offset);
        }
      );
  }

  auto State::collection_create
    ( util::not_null<StoragesClients> storages_clients
    , collection::ID collection_id
    , core::memory::Size size
    ) -> UsedStorages
  {
    if (_collections.contains (collection_id))
    {
      // \todo specific exception
      throw std::invalid_argument
       { fmt::format ( "collection_create: duplicate collection::ID: {}"
                     , collection_id
                     )
       };
    }

    return _collections.emplace
      ( collection_id
      , distribute_as_equal_as_possible
        ( storages_clients
        , size
        , core::memory::make_offset (0)
        )
      ).first->second
      ;
  }

  auto State::collection_append
    ( util::not_null<StoragesClients> storages_clients
    , collection::ID collection_id
    , core::memory::Range range_to_append
    ) -> core::memory::Size
  {
    auto& used_storages {at (collection_id)->second};
    auto const collection_range {used_storages.range()};

    using core::memory::end;

    if (end (collection_range) < end (range_to_append))
    {
      used_storages.append
        ( distribute_as_equal_as_possible
           ( storages_clients
           , end (range_to_append) - end (collection_range)
           , end (collection_range)
           )
        );
    }

    return core::memory::size (used_storages.range());
  }

  auto State::collection_delete
    ( util::not_null<StoragesClients> storages_clients
    , collection::ID collection_id
    ) -> void
  {
    auto collection {at (collection_id)};

    auto const remove_collection
      { nonstd::make_scope_exit_that_dies_on_exception
          ( "iov_backend::State::collection_delete"
          , [&]() noexcept
            {
              _collections.erase (collection);
            }
          )
      };

    storages_clients->remove (_storages, collection->second);

    return;
  }

  auto State::range
    ( collection::ID collection_id
    ) const -> core::memory::Range
  {
    return at (collection_id)->second.range();
  }

  auto State::locations
    ( collection::ID collection_id
    , core::memory::Range range
    ) const -> Locations
  {
    using core::memory::begin;
    using core::memory::end;
    using core::memory::size;

    auto const& used_storages {at (collection_id)->second};

    if (size (range) != core::memory::make_size (0))
    {
      auto const collection_range {used_storages.range()};

      if (begin (range) < begin (collection_range))
      {
        // \todo specific exception
        throw std::invalid_argument
          { fmt::format ( "locations ({}): starts before {}"
                        , range
                        , collection_range
                        )
          };
      }

      if (end (collection_range) < end (range))
      {
        // \todo specific exception
        throw std::invalid_argument
          { fmt::format ( "locations ({}): ends after {}"
                        , range
                        , collection_range
                        )
          };
      }
    }

    auto locations {Locations::ContainerOfLocation{}};

    for ( auto used_storage {used_storages.lower_bound (range)}
        ; used_storage != std::end (used_storages)
            && begin (used_storage->_range) < end (range)
        ; ++used_storage
        )
    {
      // in the first iteration that is the postcondition of
      // lower_bound, in later iterations this is guaranteed by the
      // order of the set
      assert (begin (range) < end (used_storage->_range));

      auto const intersection
        { core::memory::make_range
          ( std::max (begin (used_storage->_range), begin (range))
          , std::min (end   (used_storage->_range), end   (range))
          )
        };

      auto const& storage {_storages.at (used_storage->_storage_id)};

      locations.emplace_back
        ( intersection
        , storage._storages_provider
        , storage._storage_implementation_id
        , storage._parameter_file_read
        , storage._parameter_file_write
        , storage._transport_provider
        , core::transport::Address
          { storage._storage_id
          , storage._parameter_chunk_description
          , used_storage->_segment_id
          , core::memory::make_offset
            ( size_cast<core::memory::Offset::underlying_type>
              ( begin (intersection) - begin (used_storage->_range)
              )
            )
          }
        );
    }

    return locations;
  }

  auto State::at
    ( collection::ID collection_id
    ) const -> decltype (_collections)::const_iterator
  {
    auto collection {_collections.find (collection_id)};

    if (collection == std::end (_collections))
    {
      // \todo specific exception
      throw std::invalid_argument
        { fmt::format ("Unknown collection::ID: {}", collection_id)
        };
    }

    return collection;
  }

  auto State::at
    ( collection::ID collection_id
    ) -> decltype (_collections)::iterator
  {
    auto collection {_collections.find (collection_id)};

    if (collection == std::end (_collections))
    {
      // \todo specific exception
      throw std::invalid_argument
        { fmt::format ("Unknown collection::ID: {}", collection_id)
        };
    }

    return collection;
  }
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION3
  ( mcs::iov_backend::provider::State
  , _next_storage_id
  , _storages
  , _collections
  );

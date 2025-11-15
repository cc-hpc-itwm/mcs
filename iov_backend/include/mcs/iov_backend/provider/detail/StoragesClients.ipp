// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <mcs/core/memory/Range.hpp>
#include <mcs/rpc/multi_client/ParallelCallsLimit.hpp>
#include <mcs/rpc/multi_client/call_and_collect.hpp>
#include <mcs/util/not_null.hpp>

namespace mcs::iov_backend::provider
{
  template<typename Executor>
    auto StoragesClients::restart
      ( Executor& io_context
      , state::Storages const& storages
      , state::Collections const& collections
      ) -> void
  {
    // \todo create connections in parallel
    for (auto const& [storage_id, storage] : storages)
    {
      try
      {
        if (storage_id != add (io_context, storage_id, storage))
        {
          throw std::logic_error {"Inconsistent add response. :o"};
        }
      }
      catch (...)
      {
        // \todo specific exception
        std::throw_with_nested
          ( std::runtime_error
              { fmt::format ( "Could not contact storage {}/{}"
                            , storage_id
                            , storage._storages_provider
                            )
              }
          );
      }
    }

    chunk_description<core::chunk::access::Const> (storages, collections);
  }

  template<typename Executor>
    auto StoragesClients::add
      ( Executor& io_context
      , storage::ID storage_id
      , Storage const& storage
      ) -> storage::ID
  {
    if (_clients_by_storage_id.contains (storage_id))
    {
      // \todo specific exception
      throw std::invalid_argument
        { fmt::format
            ( "mcs::iov_backend::provider::StoragesClients"
              ": Duplicate storage::ID: '{}'"
            , storage_id
            )
        };
    }

    auto [pos, inserted]
      { _clients_by_storage_id.emplace
        ( storage_id
        , util::ASIO::run
          ( storage._storages_provider
          , [&]<util::ASIO::is_protocol Protocol>
              (util::ASIO::Connectable<Protocol> provider) -> AnyClient
            {
              return core::control::Client
                  < Protocol
                  , AccessPolicy
                  , SupportedStorageImplementations
                  >
                { io_context
                , provider
                };
            }
          )
        )
      };

    assert (inserted);

    return pos->first;
  }
}

namespace mcs::iov_backend::provider
{
  template<typename MakeRange>
    requires
    ( std::regular_invocable<MakeRange, storage::ID, Storage const&>
      &&
      std::is_same_v
        < std::invoke_result_t<MakeRange, storage::ID, Storage const&>
        , core::memory::Range
        >
    )
    auto StoragesClients::create
      ( state::Storages const& storages
      , MakeRange make_range
      ) -> UsedStorages
  {
    using SegmentCreate = core::control::command::segment::Create;

    auto used_storages_constructor {UsedStorages::Constructor{}};
    auto errors {std::unordered_map<storage::ID, std::exception_ptr>{}};
    auto ranges {std::unordered_map<storage::ID, core::memory::Range>{}};

    struct Collector
    {
      auto result ( state::Storages::value_type const& storage
                  , SegmentCreate::Response&& segment_id
                  )
      {
        _used_storages_constructor->emplace
          ( _ranges->at (storage.first)
          , segment_id
          , storage.first
          );
      }
      auto error ( state::Storages::value_type const& storage
                 , std::exception_ptr error
                 ) noexcept
      {
        _errors->emplace (storage.first, error);
      }

      util::not_null<UsedStorages::Constructor> _used_storages_constructor;
      util::not_null<std::unordered_map<storage::ID, std::exception_ptr>>
        _errors;
      util::not_null<std::unordered_map<storage::ID, core::memory::Range>>
        _ranges;
    };
    auto collector
      { Collector
          { std::addressof (used_storages_constructor)
          , std::addressof (errors)
          , std::addressof (ranges)
          }
      };

    rpc::multi_client::call_and_collect<SegmentCreate>
      ( [&] (auto const& storage)
        {
          return SegmentCreate
            { storage.second._storage_id
            , storage.second._parameter_segment_create
            , core::memory::size
              ( ranges.emplace
                ( storage.first
                , std::invoke (make_range, storage.first, storage.second)
                ).first->second
              )
            };
        }
     , [&] (auto const& storage)
       {
         return at (storage.first);
       }
     , collector
     , storages
       // \todo parameter for the ParallelCallsLimit
     , rpc::multi_client::ParallelCalls::Unlimited{}
     );

    if (!errors.empty())
    {
      // \todo rollback
      throw std::runtime_error
        { fmt::format ("StoragesClients::create: {}", errors)
        };
    }

    return used_storages_constructor;
  }
}

namespace mcs::iov_backend::provider
{
  template<core::chunk::is_access Access>
    auto StoragesClients::chunk_description
      ( state::Storages const& storages
      , state::Collections const& collections
      ) -> void
  {
    using ChunkDescription
      = SupportedStorageImplementations::template wrap
          < core::control::command::chunk::Description
          , Access
          >
      ;

    auto errors {std::unordered_map<storage::ID, std::exception_ptr>{}};

    struct Collector
    {
      auto result
        ( UsedStorage const&
        , typename ChunkDescription::Response
        ) noexcept
      {
        // ignore
        // \todo store and use the chunk descriptions
      }
      auto error ( UsedStorage const& used_storage
                 , std::exception_ptr error
                 ) noexcept
      {
        _errors->emplace (used_storage._storage_id, error);
      }
      [[nodiscard]] constexpr auto start_more_calls() const noexcept
      {
        return _errors->empty();
      }

      std::unordered_map<storage::ID, std::exception_ptr>* _errors;
    };
    auto collector {Collector {std::addressof (errors)}};

    for (auto const& [collection_id, used_storages] : collections)
    {
      rpc::multi_client::call_and_collect<ChunkDescription>
        ( [&] (auto const& used_storage)
          {
            auto const& storage {storages.at (used_storage._storage_id)};

            return ChunkDescription
              { storage._storage_id
              , storage._parameter_chunk_description
              , used_storage._segment_id
              , used_storage._range
              };
          }
        , [&] (auto const& used_storage)
          {
            return at (used_storage._storage_id);
          }
        , collector
        , used_storages
          // \todo parameter for the ParallelCallsLimit
        , rpc::multi_client::ParallelCalls::Unlimited{}
        );
    }

    if (!errors.empty())
    {
      throw std::runtime_error
        { fmt::format ("StoragesClients::chunk_description: {}", errors)
        };
    }
  }
}

namespace mcs::iov_backend::provider
{
  template<is_capacity_sink CapacitySink>
    auto StoragesClients::capacities
      ( state::Storages const& storages
      , CapacitySink capacity_sink
      ) -> void
  {
    using StorageSize = core::control::command::storage::Size;

    auto errors {std::unordered_map<storage::ID, std::exception_ptr>{}};

    struct Collector
    {
      auto result ( state::Storages::value_type const& storage_id_and_storage
                  , StorageSize::Response size
                  )
      {
        std::invoke ( *_capacity_sink
                    , storage_id_and_storage.first
                    , size.max() - size.used()
                    );
      }
      auto error ( state::Storages::value_type const& storage_id_and_storage
                 , std::exception_ptr error
                 ) noexcept
      {
        _errors->emplace (storage_id_and_storage.first, error);
      }

      util::not_null<CapacitySink> _capacity_sink;
      util::not_null<std::unordered_map<storage::ID, std::exception_ptr>>
        _errors
        ;
    };
    auto collector
      { Collector
        { std::addressof (capacity_sink)
        , std::addressof (errors)
        }
      };

    rpc::multi_client::call_and_collect<StorageSize>
      ( [&] (auto const& storage_id_and_storage)
        {
          return StorageSize
            { storage_id_and_storage.second._storage_id
            , storage_id_and_storage.second._parameter_size_max
            , storage_id_and_storage.second._parameter_size_used
            };
        }
      , [&] (auto const& storage_id_and_storage)
        {
          return at (storage_id_and_storage.first);
        }
      , collector
      , storages
        // \todo parameter for the ParallelCallsLimit
      , rpc::multi_client::ParallelCalls::Unlimited{}
      );

    if (!errors.empty())
    {
      throw std::runtime_error
        { fmt::format ("StoragesClients::capacities: {}", errors)
        };
    }
  }
}

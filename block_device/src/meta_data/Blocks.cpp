// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <fmt/format.h>
#include <iterator>
#include <mcs/block_device/meta_data/Blocks.hpp>
#include <mcs/block_device/storage/with_range.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <mutex>

namespace mcs::block_device::meta_data
{
  Blocks::Error::BlockNotInAnyStorage::BlockNotInAnyStorage (block::ID block_id)
    : mcs::Error
      { fmt::format
        ( "mcs::block_device::meta_data::Blocks::BlockNotInAnyStorage: {}\n"
        , block_id
        )
      }
    , _block_id {block_id}
  {}
  Blocks::Error::BlockNotInAnyStorage::~BlockNotInAnyStorage() = default;

  Blocks::Blocks (block::Size block_size) noexcept
    : _block_size {block_size}
  {}
  auto Blocks::number_of_blocks() const noexcept -> block::Count
  {
    auto const lock {std::shared_lock {_guard}};

    return _number_of_blocks;
  }

  auto Blocks::blocks() const -> std::list<block::Range>
  {
    auto const lock {std::shared_lock {_guard}};

    auto ranges {std::list<block::Range>{}};

    for (auto const& used_storage : _used_storages)
    {
      if (!ranges.empty() && ranges.back().is_extended_by (used_storage.range))
      {
        ranges.back().extend_by (used_storage.range);
      }
      else
      {
        ranges.emplace_back (used_storage.range);
      }
    }

    return ranges;
  }

  auto Blocks::add (Storage storage) -> AddResult
  {
    auto const number_of_blocks
      {block::make_count (size (storage._range) / _block_size)};

    if (! (number_of_blocks > block::make_count (0)))
    {
      return AddResult {.blocks={}, .unused = storage};
    }

    auto const lock {std::unique_lock {_guard}};

    auto const increment_number_of_blocks
      { nonstd::make_scope_success
          ([&]() noexcept { _number_of_blocks += number_of_blocks; })
      };

    auto const increment_block_id
      { nonstd::make_scope_success
          ([&]() noexcept { _next_block_id += number_of_blocks; })
      };

    auto const used_offset_end {block_start (storage, number_of_blocks)};

    return AddResult
      { .blocks =
          { _used_storages.emplace
              ( block::make_range (_next_block_id, number_of_blocks)
              , storage::with_range
                ( storage
                , begin (storage._range)
                , used_offset_end
                )
              ).first->range
          }
      , .unused = used_offset_end < end (storage._range)
            ? std::optional<Storage>
              { storage::with_range
                ( storage
                , used_offset_end
                , end (storage._range)
                )
              }
            : std::optional<Storage>{}
      };
  }

  auto Blocks::remove
    ( block::ID block_begin
    , block::ID const block_end
    ) -> RemoveResult
  {
    auto const lock {std::unique_lock {_guard}};

    auto storage {_used_storages.upper_bound (block_begin)};

    auto const block_intersects_storage
      { [&]() -> std::optional<block::Range>
        {
          if (storage != std::end (_used_storages))
          {
            auto const ibegin {std::max (block_begin, begin (storage->range))};
            auto const iend {std::min (block_end, end (storage->range))};

            if (ibegin < iend)
            {
              return block::UNSAFE_make_range (ibegin, iend);
            }
          }

          return {};
        }
      };

    auto result {RemoveResult{}};

    while (auto intersection {block_intersects_storage()})
    {
      if ( begin (storage->range)
         < begin (*intersection)
         )
      {
        _used_storages.emplace
          ( block::UNSAFE_make_range
             ( begin (storage->range)
             , begin (*intersection)
             )
          , storage::with_range
             ( storage->storage
             , begin (storage->storage._range)
             , block_offset (storage, begin (*intersection))
             )
          );
      }

      if ( end (*intersection)
         < end (storage->range)
         )
      {
        _used_storages.emplace
          ( block::UNSAFE_make_range
              ( end (*intersection)
              , end (storage->range)
              )
          , storage::with_range
              ( storage->storage
              , block_offset (storage, end (*intersection))
              , end (storage->storage._range)
              )
          );
      }

      result.unused.emplace_back
        ( storage::with_range
            ( storage->storage
            , block_offset (storage, begin (*intersection))
            , block_offset (storage, end (*intersection))
            )
        );

      block_begin = end (storage->range);

      storage = _used_storages.erase (storage);

      _number_of_blocks -= end (*intersection) - begin (*intersection);
    }

    return result;
  }

  auto Blocks::remove (block::Range block_range) -> RemoveResult
  {
    return remove (begin (block_range), end (block_range));
  }

  auto Blocks::location (block::ID block_id) const -> Location
  {
    auto const lock {std::shared_lock {_guard}};

    // upper_bound returns the first storage (if any) where the
    // range.end is greater than the block_id, according to the
    // UsedStorageCompare predicate. checking that block_id is not
    // smaller than the begin of the upper-storage implies the block
    // is in the upper-storage. if block_id is smaller than the begin
    // of the upper-storage then no storage contains the block or else
    // that storage would have an range.end smaller than the range.end
    // of the upper-storage and still larger than the block_id and
    // contradicting the post-condition of upper_bound.
    //
    if ( auto storage {_used_storages.upper_bound (block_id)}
       ; storage != std::end (_used_storages)
       && block_id >= begin (storage->range)
       )
    {
      return Location
        { storage->storage._provider_connectable
        , core::transport::Address
          { storage->storage._storage_id
          , storage->storage._storage_parameter_chunk_description
          , storage->storage._segment_id
          , block_offset (storage, block_id)
          }
        };
    }

    throw Error::BlockNotInAnyStorage {block_id};
  }

  auto Blocks::block_start
    ( Storage const& storage
    , block::Count b
    ) const noexcept -> core::memory::Offset
  {
    return begin (storage._range) + b * _block_size;
  }

  auto Blocks::block_offset
    ( UsedStorages::iterator storage
    , block::ID block_id
    ) const noexcept -> core::memory::Offset
  {
    return block_start (storage->storage, block_id - begin (storage->range));
  }
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2
  ( mcs::block_device::meta_data::Blocks::AddResult
  , blocks
  , unused
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::block_device::meta_data::Blocks::RemoveResult
  , unused
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2
  ( mcs::block_device::meta_data::Blocks::Location
  , provider
  , address
  );

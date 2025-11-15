// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <list>
#include <mcs/Error.hpp>
#include <mcs/block_device/Storage.hpp>
#include <mcs/block_device/block/Count.hpp>
#include <mcs/block_device/block/ID.hpp>
#include <mcs/block_device/block/Range.hpp>
#include <mcs/block_device/block/Size.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/serialization/STD/list.hpp>
#include <mcs/serialization/STD/optional.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <optional>
#include <set>
#include <shared_mutex>
#include <type_traits>

namespace mcs::block_device::meta_data
{
  struct Blocks
  {
    // Creates Blocks with a certain block size.
    //
    // Note: There is no way to change the block size after
    // creation. All blocks will have this block size.
    //
    // Complexity: O(1)
    //
    [[nodiscard]] Blocks (block::Size) noexcept;

    // Returns: The block_size.
    //
    // Complexity: O(1)
    //
    [[nodiscard]] auto constexpr block_size() const noexcept -> block::Size;

    // Returns: The number of blocks.
    //
    // Note: Valid blocks ids might be larger than number_of_blocks as
    // removal of storage can create holes, use blocks() to retrieve a
    // list of all valid block ids.
    //
    // Complexity: O(1)
    //
    [[nodiscard]] auto number_of_blocks() const noexcept -> block::Count;

    // Returns: A list of all known block ids.
    //
    // Note: Touching ranges are merged into a single range. The
    // number of ranges returned by this function might be smaller
    // than the number of storages added.
    //
    // Complexity: O(#used_storages)
    //
    [[nodiscard]] auto blocks() const -> std::list<block::Range>;

    // Adds a block storage. The memory range is used from left to
    // right in multiples of block_size(). If the storage provides
    // memory that is no a multipe of block_size(), then the unused
    // part is returned to the caller.
    //
    // Note:
    // - New block ids are greater than all existing block ids.
    // - number_of_blocks() might increase.
    //
    // Returns:
    // - The new blocks ids (if there are any).
    // - The unused storage (if there is any).
    //
    // Note: Adding the same storage multiple times is possible and
    // leads to different ids for the same physical blocks -> no
    // attempt is made to detect that situation.
    //
    // Complexity: O(log #used_storages)
    //
    struct AddResult
    {
      std::optional<block::Range> blocks{};
      std::optional<Storage> unused{};
    };
    [[nodiscard]] auto add (Storage) -> AddResult;

    // Removes a range of blocks.
    //
    // Note: A block id, once removed, never again comes back to live,
    // adding removed storage results in new block ids.
    //
    // Note: To remove a range of blocks from the middle of a storage
    // might _increase_ the number of used storages by one.
    //
    // Complexity: O(#touched storages * log #used_storages)
    //
    struct RemoveResult
    {
      std::list<Storage> unused;
    };
    auto remove (block::Range) -> RemoveResult;

    // Returns: The location of the block with the block id.
    //
    // Requires: block id `element_of` blocks()
    //
    // Complexity: O(log #used_storages)
    //
    struct Location
    {
      util::ASIO::AnyConnectable provider;
      core::transport::Address address;
    };
    [[nodiscard]] auto location (block::ID) const -> Location;

    struct Error
    {
      struct BlockNotInAnyStorage : public mcs::Error
      {
        constexpr auto block_id() const noexcept;

        MCS_ERROR_COPY_MOVE_DEFAULT (BlockNotInAnyStorage);

      private:
        friend struct Blocks;

        BlockNotInAnyStorage (block::ID);

        block::ID _block_id;
      };
    };

  private:
    block::Size _block_size;
    struct UsedStorage
    {
      block::Range range;
      Storage storage;
    };
    struct UsedStorageCompare
    {
      using is_transparent = std::true_type;

      // used to sort the set
      [[nodiscard]] constexpr auto operator()
        ( UsedStorage const&
        , UsedStorage const&
        ) const noexcept -> bool
        ;
      // used to upper_bound a block id
      [[nodiscard]] constexpr auto operator()
        ( block::ID const&
        , UsedStorage const&
        ) const noexcept -> bool
        ;
    };
    mutable std::shared_mutex _guard{};
    block::Count _number_of_blocks {block::make_count (0)};
    block::ID _next_block_id {block::make_id (0)};
    // \note on the data structure choice: iterator stability after
    // emplace and erase is exploited in remove
    using UsedStorages = std::set<UsedStorage, UsedStorageCompare>;
    UsedStorages _used_storages{};

    [[nodiscard]] auto remove (block::ID, block::ID) -> RemoveResult;
    [[nodiscard]] auto block_start
      ( Storage const&
      , block::Count
      ) const noexcept -> core::memory::Offset;
    [[nodiscard]] auto block_offset
      ( UsedStorages::iterator
      , block::ID
      ) const noexcept -> core::memory::Offset;
  };
}

namespace mcs::serialization
{
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    (block_device::meta_data::Blocks::AddResult);
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    (block_device::meta_data::Blocks::RemoveResult);
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    (block_device::meta_data::Blocks::Location);
}

#include "detail/Blocks.ipp"

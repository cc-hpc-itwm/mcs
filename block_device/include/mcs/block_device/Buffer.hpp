// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <condition_variable>
#include <mcs/Error.hpp>
#include <mcs/block_device/Block.hpp>
#include <mcs/block_device/block/Size.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <mutex>
#include <stack>
#include <variant>

namespace mcs::block_device
{
  // A Buffer for blocks. Used to buffer blocks from remote locations
  // in local memory.
  //
  template<typename StorageImplementations> struct Buffer;

  template<core::storage::is_implementation... StorageImplementations>
    struct Buffer<util::type::List<StorageImplementations...>>
  {
    using SupportedStorageImplementations
      = util::type::List<StorageImplementations...>
      ;

    Buffer ( util::not_null<core::Storages<SupportedStorageImplementations>>
           , core::storage::ID
           , core::storage::Parameter
           , core::storage::segment::ID
           , core::memory::Range
           , block_device::block::Size
           );

    // Deleter to tell the buffer about Blocks are no longer in use.
    //
    struct ReleaseOffset
    {
      auto operator()
        ( Block<SupportedStorageImplementations>*
        ) const noexcept -> void;

    private:
      friend struct Buffer<SupportedStorageImplementations>;

      ReleaseOffset (util::not_null<Buffer<SupportedStorageImplementations>>);

      util::not_null<Buffer<SupportedStorageImplementations>> _buffer;
    };

    // Context to be able to interrupt calls to block()
    //
    struct InterruptionContext
    {
      constexpr explicit InterruptionContext() noexcept = default;

    private:
      friend struct Buffer<SupportedStorageImplementations>;

      bool _interrupted {false};
    };

    // Interrupt all ongoing calls that use the interruption context.
    //
    // Note: Sticky: If an interruption context has been interrupted,
    // then it will stay interrupted. Using it in a later call will
    // interrupt immediately.
    //
    auto interrupt (util::not_null<InterruptionContext>) -> void;

    struct Error
    {
      struct Block
      {
        struct Interrupted : public mcs::Error
        {
          Interrupted();
          MCS_ERROR_COPY_MOVE_DEFAULT (Interrupted);
        };
        struct Timeout : public mcs::Error
        {
          Timeout();
          MCS_ERROR_COPY_MOVE_DEFAULT (Timeout);
        };
      };
    };

    using BufferedBlock
      = std::unique_ptr<Block<SupportedStorageImplementations>, ReleaseOffset>;

    // Returns: An unused block.
    //
    // Note: The function call blocks if no block is available. It
    // blocks no longer than the parameter time_point has been
    // exceeded or if interrupted via the interruption context.
    // If the timeout has been reached, then "Timeout" is thrown.
    // If the function is interrupted, then "Interrupted" is thrown.
    //
    template<typename TimePoint>
      auto block
        ( InterruptionContext const&
        , TimePoint const&
        ) -> BufferedBlock
      ;

  private:
    friend struct ReleaseOffset;

    util::not_null<core::Storages<SupportedStorageImplementations>> _storages;
    core::storage::ID _storage_id;
    core::storage::Parameter _storage_parameter_chunk_description;
    core::storage::segment::ID _segment_id;
    core::memory::Size _block_size;
    std::mutex _guard;
    std::condition_variable _block_available_or_interrupted;
    std::stack<core::memory::Offset> _available;
  };
}

#include "detail/Buffer.ipp"

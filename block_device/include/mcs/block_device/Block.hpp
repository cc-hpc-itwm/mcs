// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::block_device
{
  // A block in local memory.
  //
  template<typename StorageImplementations> struct Block;

  template<core::storage::is_implementation... StorageImplementations>
    struct Block<util::type::List<StorageImplementations...>>
  {
    using SupportedStorageImplementations
      = util::type::List<StorageImplementations...>
      ;

    Block ( util::not_null<core::Storages<SupportedStorageImplementations>>
          , core::storage::ID
          , core::storage::Parameter
          , core::storage::segment::ID
          , core::memory::Range
          );

    auto address() const -> core::transport::Address;

    template<core::chunk::is_access Access>
      using CoreChunk = typename SupportedStorageImplementations::template wrap
        < core::Chunk
        , Access
        >
      ;

    template<core::chunk::is_access Access>
      [[nodiscard]] auto chunk() const -> CoreChunk<Access>;

  private:
    util::not_null<core::Storages<SupportedStorageImplementations>> _storages;
    core::storage::ID _storage_id;
    core::storage::Parameter _storage_parameter_chunk_description;
    core::storage::segment::ID _segment_id;
    core::memory::Range _range;
  };
}

#include "detail/Block.ipp"

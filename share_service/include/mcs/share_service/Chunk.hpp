// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::share_service
{
  // Self contained description of a piece of memory.
  //
  struct Chunk
  {
    util::ASIO::AnyConnectable provider_connectable;
    core::storage::ID storage_id;
    SupportedStorageImplementations::ID storage_implementation_id;
    core::storage::segment::ID segment_id;
    core::memory::Size size;
  };
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::share_service::Chunk);

#include "detail/Chunk.ipp"

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::core::transport
{
  struct Address
  {
    storage::ID storage_id;
    storage::Parameter storage_parameter_chunk_description;
    storage::segment::ID segment_id;
    memory::Offset offset;
  };
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::core::transport::Address);

#include "detail/Address.ipp"

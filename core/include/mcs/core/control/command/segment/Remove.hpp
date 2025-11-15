// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::core::control::command::segment
{
  struct Remove
  {
    using Response = mcs::core::memory::Size;

    mcs::core::storage::ID storage_id;
    mcs::core::storage::Parameter storage_parameter;
    mcs::core::storage::segment::ID segment_id;
  };
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  (mcs::core::control::command::segment::Remove);

#include "detail/Remove.ipp"

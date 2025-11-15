// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::core::control::command::storage::size
{
  struct Max
  {
    using Response = mcs::core::storage::MaxSize;

    mcs::core::storage::ID storage_id;
    mcs::core::storage::Parameter storage_parameter;
  };
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  (mcs::core::control::command::storage::size::Max);

#include "detail/Max.ipp"

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/ID.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::core::control::command::storage
{
  struct Remove
  {
    using Response = void;

    mcs::core::storage::ID storage_id;
  };
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  (mcs::core::control::command::storage::Remove);

#include "detail/Remove.ipp"

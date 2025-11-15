// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <mcs/util/hash/declare.hpp>
#include <mcs/util/tuplish/declare.hpp>
#include <string>

namespace mcs::iov_backend::collection
{
  struct ID
  {
    explicit ID (std::string);

    auto operator<=> (ID const&) const = default;

    std::string _uuid;
  };
}

namespace std
{
  template<>
    MCS_UTIL_HASH_DECLARE (mcs::iov_backend::collection::ID);
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::iov_backend::collection::ID);

#include "detail/ID.ipp"

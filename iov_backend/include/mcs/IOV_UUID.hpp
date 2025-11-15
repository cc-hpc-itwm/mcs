// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <iov/meta.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/hash/declare.hpp>
#include <mcs/util/read/declare.hpp>
#include <type_traits>

namespace iov::meta
{
  // used in hash, serialization, read
  using UUID_FingerPrint =
    std::remove_cvref_t<decltype (std::declval<UUID>().str())>;
}

namespace std
{
  template<> MCS_UTIL_HASH_DECLARE (iov::meta::UUID);
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION (iov::meta::UUID);
}

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (iov::meta::UUID);
}

namespace mcs::util::read
{
  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    ( iov::meta::UUID
    );
}

#include "detail/IOV_UUID.ipp"

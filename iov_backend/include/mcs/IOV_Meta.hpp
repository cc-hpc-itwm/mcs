// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <iov/meta.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (iov::meta::Key);
  template<> MCS_UTIL_FMT_DECLARE (iov::meta::Name);
  template<> MCS_UTIL_FMT_DECLARE (iov::meta::Value);
}

#include "detail/IOV_Meta.ipp"

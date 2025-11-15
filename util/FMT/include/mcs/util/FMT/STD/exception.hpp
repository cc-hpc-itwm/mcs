// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <mcs/util/FMT/declare.hpp>

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (std::exception_ptr);
  template<> MCS_UTIL_FMT_DECLARE (std::exception);
}

#include "detail/exception.ipp"

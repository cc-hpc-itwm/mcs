// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/declare.hpp>

namespace mcs::util::read
{
  struct Comma{};

  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION (Comma);
}

#include "detail/Comma.ipp"

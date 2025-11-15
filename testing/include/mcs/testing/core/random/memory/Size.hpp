// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Size.hpp>
#include <mcs/testing/random/value/declare.hpp>
#include <mcs/testing/random/value/integral.hpp>

namespace mcs::testing::random
{
  template<>
    MCS_TESTING_RANDOM_DECLARE_VALUE_VIA_MAKE_FROM_UNDERLYING_TYPE
      ( core::memory::Size
      );
}

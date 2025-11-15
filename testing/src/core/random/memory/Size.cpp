// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/random/value/define.hpp>

namespace mcs::testing::random
{
  MCS_TESTING_RANDOM_DEFINE_VALUE_VIA_MAKE_FROM_UNDERLYING_TYPE
    (core::memory::Size, core::memory::make_size);
}

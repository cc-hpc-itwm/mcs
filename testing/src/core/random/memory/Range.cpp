// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <mcs/testing/core/random/memory/Range.hpp>
#include <mcs/testing/random/value/define.hpp>

namespace mcs::testing::random
{
  auto value<core::memory::Range>::operator()() -> core::memory::Range
  {
    auto x {_random_offset()};
    auto y {_random_offset()};

    return core::memory::make_range (std::min (x, y), std::max (x, y));
  }
}

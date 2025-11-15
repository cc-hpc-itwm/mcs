// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Range.hpp>
#include <mcs/testing/core/random/memory/Offset.hpp>

namespace mcs::testing::random
{
  template<>
    struct value<core::memory::Range>
  {
    auto operator()() -> core::memory::Range;

  private:
    value<core::memory::Offset> _random_offset;
  };
}

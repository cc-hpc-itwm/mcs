// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/testing/random/value.hpp>
#include <random>

namespace mcs::testing::random
{
  // produces a random boolean
  //
  template<> struct value<bool>
  {
    auto operator()() -> bool;

  private:
    std::uniform_int_distribution<int> _value {0, 1};
  };
}

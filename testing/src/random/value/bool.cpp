// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/random/random_device.hpp>
#include <mcs/testing/random/value/bool.hpp>

namespace mcs::testing::random
{
  auto value<bool>::operator()() -> bool
  {
    return _value (random_device()) == 1;
  }
}

// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <gtest/gtest.h>

namespace mcs::testing::random
{
  // prints the random seed on failure
  //
  struct Test : virtual ::testing::Test
  {
    ~Test() noexcept override;

    using ::testing::Test::Test;
    Test (Test const&) = delete;
    Test (Test&&) = delete;
    auto operator= (Test const&) -> Test& = delete;
    auto operator= (Test&&) -> Test& = delete;
  };
}

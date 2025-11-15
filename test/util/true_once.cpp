// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/util/true_once.hpp>

namespace mcs::util
{
  TEST (TrueOnce, is_true_when_observed_the_first_time)
  {
    auto const x {TrueOnce{}};

    ASSERT_TRUE (x);
  }

  TEST (TrueOnce, is_not_true_when_observed_the_second_or_more_times)
  {
    auto const x {TrueOnce{}};

    ASSERT_TRUE (x);

    for (auto i {0}; i != 10; ++i)
    {
      ASSERT_FALSE (x);
    }
  }
}

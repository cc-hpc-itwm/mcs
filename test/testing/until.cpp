// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/until.hpp>

namespace mcs::testing
{
  struct Until : public testing::random::Test{};

  TEST_F (Until, returns_first_return_value_that_fulfills_predicate)
  {
    auto i {0};
    auto fun {[&] { return i++; }};
    auto const wanted {testing::random::value<int> {0, 100000}()};
    auto pred {[&] (auto n) { return n == wanted; }};

    ASSERT_EQ (until (pred, fun), wanted);
  }
}

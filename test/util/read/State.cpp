// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/util/read/State.hpp>

namespace mcs::util::read
{
  namespace
  {
    auto empty()
    {
      return std::string{};
    }

    struct UtilReadStateR : public testing::random::Test
    {
      using RandomString = testing::random::value<std::string>;
    };
  }

  TEST (UtilReadState, empty_is_at_end)
  {
    auto input {empty()};

    ASSERT_TRUE (make_state (input).end());
  }

  TEST (UtilReadState, nullptr_is_at_end)
  {
    ASSERT_TRUE (make_state<char> (nullptr).end());
  }

  TEST_F (UtilReadStateR, peek_and_skip_can_be_used_to_iterate)
  {
    auto const input {RandomString{}()};
    auto state {make_state (input)};

    auto s {std::string{}};

    while (!state.end())
    {
      s += state.peek();
      state.skip();
    }

    ASSERT_EQ (input, s);
  }

  TEST_F (UtilReadStateR, head_can_be_used_to_iterate)
  {
    auto const input {RandomString{}()};
    auto state {make_state (input)};

    auto s {std::string{}};

    while (!state.end())
    {
      s += state.head();
    }

    ASSERT_EQ (input, s);
  }
}

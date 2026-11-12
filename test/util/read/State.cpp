// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <array>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/read/ParseError.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read/read.hpp>
#include <string>
#include <string_view>

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

  TEST (UtilReadState, string_view_preserves_its_bounds)
  {
    auto const input {std::array {'x', '\0', 'y', 'z'}};
    auto state {make_state (std::string_view {input.data(), 3})};

    EXPECT_EQ (state.head(), 'x');
    EXPECT_EQ (state.head(), '\0');
    EXPECT_EQ (state.head(), 'y');
    EXPECT_TRUE (state.end());
  }

  TEST (UtilReadState, empty_string_view_supports_error_reporting)
  {
    auto const input {std::string_view{}};
    auto const state {make_state (input)};

    EXPECT_TRUE (state.end());
    testing::require_exception
      ( [&]
        {
          throw state.error ("expected input");
        }
      , testing::assert_type_and_what<ParseError>
          ("ParseError:\n\n^\n: expected input")
      );
  }

  TEST (UtilReadState, read_accepts_bounded_string_views)
  {
    auto const input {std::string_view {"42trailing", 2}};

    EXPECT_EQ (read<int> (input), 42);
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

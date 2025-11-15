// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/testing/until.hpp>
#include <mcs/util/isspace.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace
{
  auto empty()
  {
    return std::string{};
  }

  struct ReadPrefixR : public mcs::testing::random::Test
  {
    using RandomString = mcs::testing::random::value<std::string>;
    using RandomPosition = mcs::testing::random::value<std::string::size_type>;

    [[nodiscard]] auto remove_whitespace_prefix (std::string s) const
    {
      return std::string
        ( std::begin (s)
        , s.erase
          ( std::begin (std::ranges::remove_if (s, mcs::util::isspace<char>))
          , std::end (s)
          )
        )
        ;
    }

    [[nodiscard]] auto random_split_position
      ( std::string const& s
      ) const noexcept
    {
      auto const position {RandomPosition {RandomPosition::Max {s.size()}}()};

      return position;
    }

    [[nodiscard]] auto random_split (std::string const& s) const
    {
      auto const position {random_split_position (s)};

      return std::make_tuple
        ( s.substr (0, position)
        , remove_whitespace_prefix (s.substr (position))
        );
    }
  };
}

namespace mcs::util::read
{
  TEST (ReadPrefix, empty_prefix_of_empty_state_does_nothing)
  {
    auto input {empty()};
    auto state {make_state (input)};
    auto prefix {empty()};

    util::read::prefix (state, prefix);
  }

  TEST_F (ReadPrefixR, full_input_is_prefix_and_skips_everything)
  {
    auto const input {remove_whitespace_prefix (RandomString{}())};
    auto state {make_state (input)};
    auto const& prefix {input};

    util::read::prefix (state, prefix);
    ASSERT_TRUE (state.end());
  }

  TEST_F (ReadPrefixR, prefix_of_input_is_prefix_and_skips_prefix)
  {
    auto const input {remove_whitespace_prefix (RandomString{}())};
    auto state {make_state (input)};
    auto const [prefix, rest] {random_split (input)};

    util::read::prefix (state, prefix);
    ASSERT_TRUE (!state.end() || rest.empty());

    util::read::prefix (state, rest);
    ASSERT_TRUE (state.end());
  }

  TEST_F (ReadPrefixR, nonempty_prefix_of_empty_state_throws)
  {
    auto const input {empty()};
    auto state {make_state (input)};
    auto const prefix {RandomString {RandomString::Length {.min = 1}}()};

    testing::require_exception
      ( [&]
        {
          util::read::prefix (state, prefix);
        }
      , testing::assert_type_and_what<ParseError>
          (fmt::format ("ParseError:\n\n^\n: Expected '{}'.", prefix))
      );
  }

  TEST_F (ReadPrefixR, not_a_prefix_throws)
  {
    auto random_string {RandomString{}};
    auto const input {random_string()};
    auto state {make_state (input)};
    auto const not_a_prefix
      { testing::until
          ( [&] (auto const& s)
            {
              return !input.starts_with (s);
            }
          , [&]
            {
              return random_string();
            }
          )
      };

    testing::require_exception
      ( [&]
        {
          util::read::prefix (state, not_a_prefix);
        }
      , testing::assert_type_and_what<ParseError>
          (fmt::format ( "ParseError:\n{}\n^\n: Expected '{}'."
                       , input
                       , not_a_prefix
                       )
          )
      );
  }
}

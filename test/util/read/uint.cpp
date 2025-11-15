// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/isdigit.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>
#include <string>
#include <tuple>

namespace
{
  struct ReadOfFmtIsIdentity : public mcs::testing::random::Test{};
}

TEST_F (ReadOfFmtIsIdentity, uint)
{
  mcs::testing::read_of_fmt_is_identity
    (mcs::testing::random::value<unsigned int>{}());
}

namespace mcs::util::read
{
  namespace
  {
    struct ReadUInt : public testing::random::Test
    {
      using RandomString = testing::random::value<std::string>;

      [[nodiscard]] auto random_string_without_leading_digit() const
      {
        auto s {RandomString{}()};

        return std::string
          ( std::begin (s)
          , s.erase
            ( std::begin (std::ranges::remove_if (s, mcs::util::isdigit<char>))
            , std::end (s)
            )
          )
          ;
      }

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
    };
  }

  TEST_F (ReadUInt, empty_state_throws)
  {
    auto const input {std::string{}};

    testing::require_exception
      ( [&]
        {
          std::ignore = read<unsigned int> (input);
        }
      , testing::assert_type_and_what<ParseError>
          ("ParseError:\n\n^\n: Expected digit.")
      );
  }

  TEST_F (ReadUInt, leading_non_digit_throws)
  {
    auto const input {random_string_without_leading_digit()};

    testing::require_exception
      ( [&]
        {
          std::ignore = read<unsigned int> (input);
        }
      , testing::assert_type_and_what<ParseError>
          (fmt::format ("ParseError:\n{}\n^\n: Expected digit.", input))
      );
  }

  TEST_F (ReadUInt, read_consumes_longest_prefix)
  {
    auto const value {testing::random::value<unsigned int>{}()};
    auto const number {fmt::format ("{}", value)};
    auto const suffix
      {remove_whitespace_prefix (random_string_without_leading_digit())};
    auto const input {number + suffix};
    auto state {make_state (input)};

    ASSERT_EQ (parse<unsigned int> (state), value);

    prefix (state, suffix);

    ASSERT_TRUE (state.end());
  }
}

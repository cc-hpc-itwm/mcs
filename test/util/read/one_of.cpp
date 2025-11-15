// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/random_device.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/isspace.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/read/one_of.hpp>
#include <mcs/util/read/prefix.hpp>
#include <string>
#include <tuple>

namespace mcs::util::read
{
  namespace
  {
    struct ReadOneOf : public testing::random::Test
    {
      using RandomString = testing::random::value<std::string>;

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

  TEST_F (ReadOneOf, empty_allowed_throws)
  {
    auto const input {RandomString{}()};
    auto const allowed {std::string{}};
    auto state {make_state (input)};

    testing::require_exception
      ( [&]
        {
          std::ignore = one_of (state, allowed);
        }
      , testing::assert_type_and_what<ParseError>
          (fmt::format ("ParseError:\n{}\n^\n: Expected one of ''.", input))
      );
  }

  TEST_F (ReadOneOf, not_starting_with_allowed_throws)
  {
    auto random_string {RandomString{}};
    auto const input {random_string()};
    auto const allowed
      { [&]
        {
          auto xs {random_string()};

          if (!input.empty())
          {
            std::erase (xs, input.at (0));
          }

          return xs;
        }()
      };
    auto state {make_state (input)};

    testing::require_exception
      ( [&]
        {
          std::ignore = one_of (state, allowed);
        }
      , testing::assert_type_and_what<ParseError>
          (fmt::format ( "ParseError:\n{}\n^\n: Expected one of '{}'."
                       , input
                       , allowed
                       )
          )
      );
  }

  TEST_F (ReadOneOf, the_found_character_is_returned)
  {
    auto const input
      { remove_whitespace_prefix
          (RandomString {RandomString::Length {.min = 1}}())
      };
    auto const allowed
      { [&]
        {
          auto xs {RandomString{}()};
          xs += input.at (0);

          std::ranges::shuffle (xs, testing::random::random_device());

          return xs;
        }()
      };
    auto state {make_state (input)};

    ASSERT_EQ (input.at (0), one_of (state, allowed));

    auto rest {std::string (std::begin (input) + 1, std::end (input))};

    prefix (state, remove_whitespace_prefix (rest));

    ASSERT_TRUE (state.end());
  }
}

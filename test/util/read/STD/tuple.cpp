// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/random/value/UTIL/string.hpp>
#include <mcs/testing/random/value/char.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/isdigit.hpp>
#include <mcs/util/isspace.hpp>
#include <mcs/util/read/STD/tuple.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/string.hpp>
#include <string>
#include <tuple>

namespace mcs::util::read
{
  TEST (ReadOfFmtIsIdentity, empty_tuple)
  {
    testing::read_of_fmt_is_identity (std::make_tuple());
  }

  namespace
  {
    using Types = ::testing::Types
      < unsigned int
      , util::string
      >;
    template<class> struct ReadOfFmtIsIdentityT : public testing::random::Test{};
    TYPED_TEST_SUITE (ReadOfFmtIsIdentityT, Types);
  }

  TYPED_TEST (ReadOfFmtIsIdentityT, singleton_tuple)
  {
    auto const value {testing::random::value<TypeParam>{}()};

    testing::read_of_fmt_is_identity (std::make_tuple (value));
  }

  namespace
  {
    struct ReadOfFmtIsIdentityR : public testing::random::Test{};
  }

  TEST_F (ReadOfFmtIsIdentityR, tuple_uint)
  {
    auto random_uint {testing::random::value<unsigned int>{}};

    testing::read_of_fmt_is_identity (std::make_tuple (random_uint()));
  }

  TEST_F (ReadOfFmtIsIdentityR, tuple_util_string)
  {
    auto random_string {testing::random::value<util::string>{}};

    testing::read_of_fmt_is_identity (std::make_tuple (random_string()));
  }

  TEST_F (ReadOfFmtIsIdentityR, tuple_uint_uint)
  {
    auto random_uint {testing::random::value<unsigned int>{}};

    testing::read_of_fmt_is_identity
      (std::make_tuple (random_uint(), random_uint()));
  }

  TEST_F (ReadOfFmtIsIdentityR, tuple_uint_util_string)
  {
    auto random_uint {testing::random::value<unsigned int>{}};
    auto random_string {testing::random::value<util::string>{}};

    testing::read_of_fmt_is_identity
      (std::make_tuple (random_uint(), random_string()));
  }

  TEST_F (ReadOfFmtIsIdentityR, tuple_util_string_uint)
  {
    auto random_string {testing::random::value<util::string>{}};
    auto random_uint {testing::random::value<unsigned int>{}};

    testing::read_of_fmt_is_identity
      (std::make_tuple (random_string(), random_uint()));
  }

  TEST_F (ReadOfFmtIsIdentityR, tuple_util_string_util_string)
  {
    auto random_string {testing::random::value<util::string>{}};

    testing::read_of_fmt_is_identity
      (std::make_tuple (random_string(), random_string()));
  }

  namespace
  {
    struct ReadTupleR : public testing::random::Test{};

    using Tuples = ::testing::Types
      < std::tuple<>
      , std::tuple<unsigned int>
      , std::tuple<util::string>
      , std::tuple<unsigned int, unsigned int>
      , std::tuple<unsigned int, util::string>
      , std::tuple<util::string, unsigned int>
      , std::tuple<util::string, util::string>
      >;
    template<class> struct ReadTupleT : public testing::random::Test{};
    TYPED_TEST_SUITE (ReadTupleT, Tuples);
  }

  TYPED_TEST (ReadTupleT, tuple_must_start_with_left_round_bracket)
  {
    using RandomString = testing::random::value<std::string>;
    using RandomChar = testing::random::value<char>;

    auto const prefix {std::string{}};
    auto const suffix
      { [&]
        {
          auto xs {RandomString{}()};
          auto random_char {RandomChar{}};

          while (!xs.empty() && xs.at (0) == '(')
          {
            xs[0] = random_char();
          }

          return xs;
        }()
      };
    auto const input {prefix + suffix};

    auto state {make_state (input)};

    testing::require_exception
      ( [&]
        {
          std::ignore = parse<TypeParam> (state);
        }
      , testing::assert_type_and_what<ParseError>
          (fmt::format ("ParseError:\n{}\n^\n: Expected '('.", input))
      );
  }

  TEST_F (ReadTupleR, tuple_must_end_with_right_round_bracket_empty)
  {
    using RandomString = testing::random::value<std::string>;
    using RandomChar = testing::random::value<char>;

    auto const prefix {std::string {"("}};
    auto const suffix
      { [&]
        {
          auto xs {RandomString{}()};
          auto random_char {RandomChar{}};

          while (  !xs.empty()
                && (  xs.at (0) == ')'
                   || util::isspace (xs.at (0))
                   )
                )
          {
            xs[0] = random_char();
          }

          return xs;
        }()
      };
    auto const input {prefix + suffix};

    auto state {make_state (input)};

    testing::require_exception
      ( [&]
        {
          std::ignore = parse<std::tuple<>> (state);
        }
      , testing::assert_type_and_what<ParseError>
          (fmt::format ("ParseError:\n{}\n ^\n : Expected ')'.", input))
      );
  }

  namespace
  {
    auto random_suffix_that_closes_the_tuple_but_is_not_a_right_round_bracket()
    {
      using RandomString = testing::random::value<std::string>;
      using RandomChar = testing::random::value<char>;

      auto xs {RandomString{}()};
      auto random_char {RandomChar{}};

      while (  ! xs.empty()
            && (  xs.at (0) == ')'
               || util::isdigit (xs.at (0))
               || util::isspace (xs.at (0))
               )
            )
      {
        xs[0] = random_char();
      }

      return xs;
    }
  }

  TEST_F (ReadTupleR, tuple_must_end_with_right_round_bracket_uint)
  {
    using RandomUInt = testing::random::value<unsigned int>;

    auto const prefix {fmt::format ("({}", RandomUInt{}())};
    auto const suffix
      {random_suffix_that_closes_the_tuple_but_is_not_a_right_round_bracket()};
    auto const input {prefix + suffix};

    auto state {make_state (input)};

    testing::require_exception
      ( [&]
        {
          fmt::print ("{}\n", parse<std::tuple<unsigned int>> (state));
        }
      , testing::assert_type_and_what<ParseError>
          (fmt::format ("ParseError:\n{}\n ^\n : Expected ')'.", input))
      );
  }

  TEST_F (ReadTupleR, tuple_must_end_with_right_round_bracket_uint_uint)
  {
    using RandomUInt = testing::random::value<unsigned int>;

    auto random_uint {RandomUInt{}};

    auto const prefix {fmt::format ("({},{}", random_uint(), random_uint())};
    auto const suffix
      {random_suffix_that_closes_the_tuple_but_is_not_a_right_round_bracket()};
    auto const input {prefix + suffix};

    auto state {make_state (input)};

    testing::require_exception
      ( [&]
        {
          std::ignore = parse<std::tuple<unsigned int, unsigned int>> (state);
        }
      , testing::assert_type_and_what<ParseError>
          ( fmt::format
              ( "ParseError:\n{0}\n{1}^\n{1}: Expected ')'."
              , input
              , std::string (prefix.size(), ' ')
              )
          )
      );
  }
}

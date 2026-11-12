// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fmt/format.h>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <limits>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/isdigit.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read/prefix.hpp>
#include <string>
#include <tuple>
#include <type_traits>

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
    template<typename Integer>
      struct ReadUIntBoundaries : ::testing::Test
    {};

    using UnsignedIntegerTypes = ::testing::Types
      < std::uint_least8_t
      , std::uint_least16_t
      , std::uint_least32_t
      , std::uint_least64_t
      >;

    TYPED_TEST_SUITE (ReadUIntBoundaries, UnsignedIntegerTypes);

    template<typename Integer>
      struct ReadIntBoundaries : testing::random::Test
    {};

    using SignedIntegerTypes = ::testing::Types
      < std::int_least8_t
      , std::int_least16_t
      , std::int_least32_t
      , std::int_least64_t
      >;

    TYPED_TEST_SUITE (ReadIntBoundaries, SignedIntegerTypes);

    struct ReadUInt : public testing::random::Test
    {
      using RandomString = testing::random::value<std::string>;

      [[nodiscard]] auto random_string_without_leading_digit() const
      {
        auto s {RandomString{}()};

        return std::string
          ( std::begin (s)
          , s.erase
            ( std::begin
              ( std::ranges::remove_if (s, mcs::util::isdigit<char>)
              )
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
            ( std::begin
              ( std::ranges::remove_if (s, mcs::util::isspace<char>)
              )
            , std::end (s)
            )
          )
          ;
      }
    };
  }

  TYPED_TEST (ReadIntBoundaries, read_of_fmt_is_identity)
  {
    testing::read_of_fmt_is_identity (testing::random::value<TypeParam>{}());
  }

  TYPED_TEST (ReadIntBoundaries, limits_and_prefix_are_preserved)
  {
    EXPECT_EQ (read<TypeParam> ("-0"), TypeParam {0});
    EXPECT_EQ (read<TypeParam> (" \t-000"), TypeParam {0});

    auto const values
      { std::array
        { std::numeric_limits<TypeParam>::min()
        , TypeParam {-1}
        , TypeParam {0}
        , TypeParam {1}
        , std::numeric_limits<TypeParam>::max()
        }
      };

    std::ranges::for_each
      ( values
      , [] (TypeParam value)
        {
          auto const text {fmt::format ("{}", value)};
          SCOPED_TRACE (text);

          EXPECT_EQ (read<TypeParam> (text), value);
          EXPECT_EQ
            ( read<TypeParam>
              ( text.starts_with ("-")
                ? " \t-000" + text.substr (1)
                : " \t000" + text
              )
            , value
            );

          auto const input {text + "suffix"};
          auto state {make_state (input)};

          EXPECT_EQ (parse<TypeParam> (state), value);

          prefix (state, "suffix");
          EXPECT_TRUE (state.end());
        }
      );
  }

  TYPED_TEST (ReadIntBoundaries, out_of_range_throws)
  {
    using UnsignedInteger = std::make_unsigned_t<TypeParam>;
    auto const maximum {std::numeric_limits<TypeParam>::max()};
    auto const inputs
      { std::array
        { fmt::format ("{}", maximum + UnsignedInteger {1})
        , fmt::format ("{}", maximum + UnsignedInteger {2})
        , fmt::format ("-{}", maximum + UnsignedInteger {2})
        }
      };

    std::ranges::for_each
      ( inputs
      , [&] (std::string const& input)
        {
          SCOPED_TRACE (input);

          testing::require_exception
            ( [&]
              {
                std::ignore = read<TypeParam> (input);
              }
            , testing::assert_type_and_what<util::CastOutOfRange>
              ( fmt::format
                ( "cast unsigned to signed: {} > {}"
                , input.starts_with ("-") ? input.substr (1) : input
                , maximum
                )
              )
            );
        }
      );
  }

  TYPED_TEST (ReadIntBoundaries, unsigned_magnitude_overflow_throws)
  {
    using UnsignedInteger = std::make_unsigned_t<TypeParam>;
    auto const maximum
      { fmt::format ("{}", std::numeric_limits<UnsignedInteger>::max())
      };
    auto const inputs {std::array {maximum + "0", "-" + maximum + "0"}};

    std::ranges::for_each
      ( inputs
      , [] (std::string const& input)
        {
          SCOPED_TRACE (input);

          testing::require_exception
            ( [&]
              {
                std::ignore = read<TypeParam> (input);
              }
            , testing::assert_type_and_what<ParseError>
              ( fmt::format
                ( "ParseError:\n{}\n{}^\n{}: UnsignedInteger out of range."
                , input
                , std::string (input.size(), ' ')
                , std::string (input.size(), ' ')
                )
              )
            );
        }
      );
  }

  TYPED_TEST (ReadIntBoundaries, invalid_input_throws)
  {
    struct Input
    {
      std::string text;
      std::size_t position {0};
      std::string reason;
    };

    auto const inputs
      { std::array
        { Input {"", 0, "Expected digit."}
        , Input {" \t", 2, "Expected digit."}
        , Input {"-", 1, "Expected digit."}
        , Input {" \t-", 3, "Expected digit."}
        , Input {"+1", 0, "Expected digit."}
        , Input {"--1", 1, "Expected digit."}
        , Input {"-x", 1, "Expected digit."}
        , Input {"1x", 1, "read: Additional input"}
        , Input {"-1x", 2, "read: Additional input"}
        }
      };

    std::ranges::for_each
      ( inputs
      , [] (Input const& input)
        {
          SCOPED_TRACE (input.text);

          testing::require_exception
            ( [&]
              {
                std::ignore = read<TypeParam> (input.text);
              }
            , testing::assert_type_and_what<ParseError>
              ( fmt::format
                ( "ParseError:\n{}\n{}^\n{}: {}"
                , input.text
                , std::string (input.position, ' ')
                , std::string (input.position, ' ')
                , input.reason
                )
              )
            );
        }
      );
  }

  TYPED_TEST (ReadUIntBoundaries, maximum_and_prefix_are_preserved)
  {
    auto const maximum {std::numeric_limits<TypeParam>::max()};
    auto const text {fmt::format ("{}", maximum)};

    EXPECT_EQ (read<TypeParam> (text), maximum);
    EXPECT_EQ (read<TypeParam> (" \t000" + text), maximum);

    auto const input {text + "suffix"};
    auto state {make_state (input)};

    EXPECT_EQ (parse<TypeParam> (state), maximum);

    prefix (state, "suffix");
    EXPECT_TRUE (state.end());
  }

  TYPED_TEST (ReadUIntBoundaries, overflow_throws_before_wrapping)
  {
    auto const maximum
      { fmt::format ("{}", std::numeric_limits<TypeParam>::max())
      };

    auto const one_above
      { std::invoke
        ( [&]
          {
            static_assert (std::is_unsigned_v<TypeParam>);

            // \note unsigned maxima are 2^n-1 with n > 0 which can
            // never end in "9":
            //     2^(4k+1) == 2 mod 10
            //     2^(4k+2) == 4 mod 10
            //     2^(4k+3) == 8 mod 10
            //     2^(4k+4) == 6 mod 10
            //     2^(4k+5) == 2 == 2^(4k+1) mod 10
            //
            auto _one_above {maximum};
            ++_one_above.back();
            return _one_above;
          }
        )
      };

    for (auto const& input : {one_above, maximum + "0"})
    {
      testing::require_exception
        ( [&]
          {
            std::ignore = read<TypeParam> (input);
          }
        , testing::assert_type_and_what<ParseError>
          ( fmt::format
            ( "ParseError:\n{}\n{}^\n{}: UnsignedInteger out of range."
            , input
            , std::string (input.size(), ' ')
            , std::string (input.size(), ' ')
            )
          )
        );
    }
  }

  TEST (ReadUIntBoundaries, byte_overflow_throws)
  {
    EXPECT_EQ (read<std::byte> ("255"), std::byte {255});

    testing::require_exception
      ( []
        {
          std::ignore = read<std::byte> ("256");
        }
      , testing::assert_type_and_what<ParseError>
          ( "ParseError:\n256\n   ^\n   : UnsignedInteger out of range."
          )
      );
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

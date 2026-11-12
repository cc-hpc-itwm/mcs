// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <array>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/read/split_at_last_whitespace.hpp>
#include <mcs/util/string.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>

namespace
{
  struct ReadOfFmtIsIdentityR : public mcs::testing::random::Test{};
}

TEST_F (ReadOfFmtIsIdentityR, string)
{
  mcs::testing::read_of_fmt_is_identity
    (mcs::util::string {mcs::testing::random::value<std::string>{}()});
}

TEST (ReadSplitAtLastWhitespace, preserves_path_whitespace)
{
  auto const fields
    {mcs::util::read::split_at_last_whitespace ("a path with spaces 4096")};
  EXPECT_EQ (fields.prefix, "a path with spaces");
  EXPECT_EQ (fields.suffix, "4096");

  auto const trailing
    {mcs::util::read::split_at_last_whitespace ("trailing  persistent")};
  EXPECT_EQ (trailing.prefix, "trailing ");
  EXPECT_EQ (trailing.suffix, "persistent");

  std::ranges::for_each
    ( std::array {' ', '\t', '\n', '\r', '\f', '\v'}
    , [] (char separator)
      {
        auto const text {std::string {"path"} + separator + "temporary"};
        auto const separated
          { mcs::util::read::split_at_last_whitespace (text)
          };
        EXPECT_EQ (separated.prefix, "path");
        EXPECT_EQ (separated.suffix, "temporary");
        EXPECT_EQ (separated.prefix.data(), text.data());
        EXPECT_EQ (separated.suffix.data(), text.data() + 5);
      }
    );
}

TEST (ReadSplitAtLastWhitespace, supports_bounded_input)
{
  auto const text {std::string {"a bignored"}};
  auto const input {std::string_view {text.data(), 3}};
  auto const fields {mcs::util::read::split_at_last_whitespace (input)};
  EXPECT_EQ (fields.prefix, "a");
  EXPECT_EQ (fields.suffix, "b");
  EXPECT_EQ (fields.suffix.data(), text.data() + 2);

  auto const unseparated {std::string_view {text.data(), 1}};
  mcs::testing::require_exception
    ( [&]
      {
        std::ignore = mcs::util::read::split_at_last_whitespace
          ( unseparated
          );
      }
    , mcs::testing::assert_type_and_what<std::invalid_argument>
        ("expected two nonempty fields separated by whitespace")
    );
}

TEST (ReadSplitAtLastWhitespace, rejects_missing_fields)
{
  std::ranges::for_each
    ( std::array<std::string_view, 14>
      { std::string_view{}, "", "file", " file", "file ", "file 4096 "
      , " ", "  ", "\t", "file\t", "file\n", "file\r", "file\f", "file\v"
      }
    , [] (std::string_view text)
      {
        mcs::testing::require_exception
          ( [&]
            {
              std::ignore = mcs::util::read::split_at_last_whitespace (text);
            }
          , mcs::testing::assert_type_and_what<std::invalid_argument>
              ("expected two nonempty fields separated by whitespace")
          );
      }
    );
}

// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <limits>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/divru.hpp>

namespace mcs::util
{
  namespace
  {
    struct UtilDivruR : public testing::random::Test
    {
      using RandomUnsigned = testing::random::value<unsigned>;
      using Min = typename RandomUnsigned::Min;
      using Max = typename RandomUnsigned::Max;

      static auto constexpr half {std::numeric_limits<unsigned>::max() >> 1u};
    };
  }

  TEST_F (UtilDivruR, division_by_zero_throws)
  {
    auto const v {RandomUnsigned{}()};

    testing::require_exception
      ( [&]
        {
          std::ignore = divru (v, 0u);
        }
      , testing::Assert<Divru::Error<unsigned>>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.numerator(), v);
            ASSERT_EQ (caught.denominator(), 0u);
            ASSERT_STREQ
              ( caught.what()
              , fmt::format ("divru ({}, {})", v, 0u).c_str()
              );
          }
        }
      , testing::Assert<Divru::DivisionByZero>
        { [] (auto const& caught)
          {
            ASSERT_STREQ (caught.what(), "DivisionByZero");
          }
        }
      );
  }

  TEST_F (UtilDivruR, zero_divru_some_nonzero_value_is_zero)
  {
    ASSERT_EQ (divru (0u, RandomUnsigned {Min {1u}}()), 0u);
  }

  TEST_F (UtilDivruR, one_divru_some_nonzero_value_is_one)
  {
    ASSERT_EQ (divru (1u, RandomUnsigned {Min {1u}}()), 1u);
  }

  TEST_F (UtilDivruR, some_value_divru_one_is_that_value)
  {
    auto const v {RandomUnsigned{}()};
    ASSERT_EQ (divru (v, 1u), v);
  }

  TEST_F (UtilDivruR, some_value_divru_that_value_is_one)
  {
    auto const v {RandomUnsigned {Max {half}}()};
    ASSERT_EQ (divru (v, v), 1u);
  }

  TEST_F (UtilDivruR, half_max_divru_half_max_is_one)
  {
    ASSERT_EQ (divru (half, half), 1u);
  }

  TEST_F (UtilDivruR, half_max_plus_one_divru_half_max_plus_one_is_one)
  {
    ASSERT_EQ (divru (half + 1u, half + 1u), 1u);
  }

  TEST_F (UtilDivruR, half_max_plus_one_divru_half_max_plus_two_overflows)
  {
    testing::require_exception
      ( [&]
        {
          std::ignore = divru (half + 1u, half + 2u);
        }
      , testing::Assert<Divru::Error<unsigned>>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.numerator(), half + 1u);
            ASSERT_EQ (caught.denominator(), half + 2u);
            ASSERT_STREQ
              ( caught.what()
              , fmt::format ("divru ({}, {})", half + 1u, half + 2u).c_str()
              );
          }
        }
      , testing::Assert<Divru::OverflowInIntermediateValue>
        { [] (auto const& caught)
          {
            ASSERT_STREQ (caught.what(), "OverflowInIntermediateValue");
          }
        }
      );
  }

  TEST_F (UtilDivruR, half_max_plus_two_divru_half_max_plus_one_overflows)
  {
    testing::require_exception
      ( [&]
        {
          std::ignore = divru (half + 2u, half + 1u);
        }
      , testing::Assert<Divru::Error<unsigned>>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.numerator(), half + 2u);
            ASSERT_EQ (caught.denominator(), half + 1u);
            ASSERT_STREQ
              ( caught.what()
              , fmt::format ("divru ({}, {})", half + 2u, half + 1u).c_str()
              );
          }
        }
      , testing::Assert<Divru::OverflowInIntermediateValue>
        { [] (auto const& caught)
          {
            ASSERT_STREQ (caught.what(), "OverflowInIntermediateValue");
          }
        }
      );
  }

  TEST_F (UtilDivruR, result_times_denominator_is_not_smaller_than_numerator)
  {
    auto const n {RandomUnsigned {Min {0u}, Max {half}}()};
    auto const d {RandomUnsigned {Min {1u}, Max {half}}()};
    ASSERT_GE (divru (n, d) * d, n);
  }

  TEST_F
    ( UtilDivruR
    , result_times_denominator_is_less_than_sum_of_numerator_and_denominator
    )
  {
    auto const n {RandomUnsigned {Min {0u}, Max {half}}()};
    auto const d {RandomUnsigned {Min {1u}, Max {half}}()};
    ASSERT_LT (divru (n, d) * d, n + d);
  }

  TEST_F
    ( UtilDivruR
    , result_times_denominator_plus_one_can_equal_the_sum_of_numerator_and_denominator
    )
  {
    auto const d {RandomUnsigned {Min {1u}, Max {1u << 15u}}()};
    auto const k {RandomUnsigned {Min {0u}, Max {1u << 15u}}()};
    ASSERT_GE (k * d, 0u);
    auto const n {k * d + 1u};
    ASSERT_LT (n, half + 1u);
    ASSERT_EQ (divru (n, d) * d + 1u, n + d);
  }
}

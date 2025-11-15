// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/core/memory/Range.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/testing/core/printer/memory/Range.hpp>
#include <mcs/testing/core/random/memory/Range.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/unique_values.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/testing/require_exception.hpp>

namespace
{
  struct ReadOfFmtIsIdentityR : public mcs::testing::random::Test{};
}

TEST_F (ReadOfFmtIsIdentityR, mcs_range)
{
  mcs::testing::read_of_fmt_is_identity
    (mcs::testing::random::value<mcs::core::memory::Range>{}());
}

namespace mcs::core::memory
{
  struct MCSMemoryRange : public testing::random::Test
  {
    using RandomValue = testing::random::value<Offset::underlying_type>;
  };

  TEST_F (MCSMemoryRange, is_serializable)
  {
    static_assert (serialization::is_serializable<Range>);

    testing::deserialized_from_serialized_is_identity
      (testing::random::value<Range>{}());
  }

  TEST_F (MCSMemoryRange, begin_must_not_ne_larger_than_end)
  {
    auto random_value {RandomValue{}};
    using testing::random::unique_values;
    auto xs {unique_values<Offset::underlying_type> (random_value, 2)};
    std::ranges::sort (xs);
    auto begin {make_offset (xs.at (1))};
    auto end {make_offset (xs.at (0))};

    ASSERT_GT (begin, end);

    testing::require_exception
      ( [&]
        {
          std::ignore = make_range (begin, end);
        }
      , testing::Assert<Range::Error::BeginMustNotBeLargerThanEnd>
          { [&] (auto const& caught)
            {
              ASSERT_EQ (begin, caught.begin());
              ASSERT_EQ (end, caught.end());
              ASSERT_STREQ
                ( caught.what()
                , fmt::format
                   ( "memory::Range::BeginMustNotBeLargerThanEnd: {} > {}"
                   , begin
                   , end
                   ).c_str()
                );
            }
          }
      );
  }

  TEST_F (MCSMemoryRange, size_returns_difference_of_end_and_begin)
  {
    auto random_value {RandomValue{}};
    auto x {random_value()};
    auto y {random_value()};
    auto begin {std::min (x, y)};
    auto end {std::max (x, y)};

    ASSERT_EQ (size (make_range (begin, end)), make_size (end - begin));
  }

  TEST_F (MCSMemoryRange, shift_shifts_begin_and_end)
  {
    auto random_value
      { RandomValue
        { RandomValue::Min
          { std::numeric_limits<Offset::underlying_type>::lowest()
          }
        , RandomValue::Max
          { std::numeric_limits<Offset::underlying_type>::max() / 2
          }
        }
      };
    auto const x {random_value()};
    auto const y {random_value()};
    auto const range {make_range (std::min (x, y), std::max (x, y))};
    auto const offset {make_offset (random_value())};
    auto const shifted_range {shift (range, offset)};
    ASSERT_EQ (begin (shifted_range), offset + begin (range));
    ASSERT_EQ (end (shifted_range), offset + end (range));
  }
}

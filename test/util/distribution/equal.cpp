// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/distribution/equal.hpp>

namespace mcs::util::distribution::equal
{
  namespace
  {
    struct MCSUtilDistributionEqualR : public testing::random::Test{};
  }

  TEST_F (MCSUtilDistributionEqualR, zero_elements_can_be_distributed)
  {
    auto const P {testing::random::value<unsigned int> {1U, 1'000U}()};

    for (auto slot {0U}; slot != P; ++slot)
    {
      ASSERT_EQ (begin (0U, P, slot), 0U);
      ASSERT_EQ (end (0U, P, slot), 0U);
      ASSERT_EQ (size (0U, P, slot), 0U);
    }
  }

  TEST_F (MCSUtilDistributionEqualR, ranges_cover_all_elements)
  {
    auto const P {testing::random::value<unsigned int> {1U, 1'000U}()};
    auto const M {testing::random::value<unsigned int> {0U, 1'000'000U}()};

    ASSERT_EQ (begin (M, P, 0U), 0U);
    ASSERT_EQ (end (M, P, P - 1U), M);
  }

  TEST_F (MCSUtilDistributionEqualR, begin_after_is_well_defined)
  {
    auto const P {testing::random::value<unsigned int> {1U, 1'000U}()};
    auto const M {testing::random::value<unsigned int> {0U, 1'000'000U}()};

    ASSERT_EQ (begin (M, P, P), M);
  }

  TEST_F (MCSUtilDistributionEqualR, ranges_are_touching)
  {
    auto const P {testing::random::value<unsigned int> {1U, 1'000U}()};
    auto const M {testing::random::value<unsigned int> {0U, 1'000'000U}()};

    for (auto slot {0U}; slot != P; ++slot)
    {
      ASSERT_EQ (end (M, P, slot), begin (M, P, slot + 1U));
    }
  }

  TEST_F (MCSUtilDistributionEqualR, range_sizes_differ_by_at_most_one)
  {
    auto const P {testing::random::value<unsigned int> {1U, 1'000U}()};
    auto const M {testing::random::value<unsigned int> {0U, 1'000'000U}()};

    for (auto slot_x {0U}; slot_x != P; ++slot_x)
    {
      for (auto slot_y {0U}; slot_y != P; ++slot_y)
      {
        auto const size_x {size (M, P, slot_x)};
        auto const size_y {size (M, P, slot_y)};
        auto const size_diff
          { std::max (size_x, size_y) - std::min (size_x, size_y)
          };
        ASSERT_LE (size_diff, 1U);
      }
    }
  }
}

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/core/memory/Size.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/testing/core/printer/memory/Size.hpp>
#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>

namespace
{
  struct ReadOfFmtIsIdentityR : public mcs::testing::random::Test{};
}

TEST_F (ReadOfFmtIsIdentityR, mcs_size)
{
  mcs::testing::read_of_fmt_is_identity
    (mcs::testing::random::value<mcs::core::memory::Size>{}());
}

namespace mcs::core::memory
{
  struct MCSMemorySize : public testing::random::Test
  {
    using RandomValue = testing::random::value<Size::underlying_type>;
    using Min = RandomValue::Min;
  };

  TEST_F (MCSMemorySize, is_serializable)
  {
    static_assert (serialization::is_serializable<Size>);

    testing::deserialized_from_serialized_is_identity
      (testing::random::value<Size>{}());
  }

  TEST_F (MCSMemorySize, is_readable_without_prefix)
  {
    auto const value {RandomValue{}()};
    auto const input {fmt::format ("{}", value)};
    ASSERT_EQ (make_size (value), util::read::read<Size> (input));
  }

  TEST_F (MCSMemorySize, operator_plus_equals_works)
  {
    auto random_value {RandomValue{}};
    auto x {random_value()};
    auto s {make_size (x)};
    auto const y {random_value()};
    s += make_size (y);
    x += y;

    ASSERT_EQ (s, make_size (x));
  }

  TEST_F (MCSMemorySize, operator_minus_equals_works)
  {
    auto random_value {RandomValue{}};
    auto x {random_value()};
    auto s {make_size (x)};
    auto const y {random_value()};
    s -= make_size (y);
    x -= y;

    ASSERT_EQ (s, make_size (x));
  }

  TEST_F (MCSMemorySize, operator_minus_works)
  {
    auto random_value {RandomValue{}};
    auto const x {random_value()};
    auto const y {random_value()};

    ASSERT_EQ (make_size (x) - make_size (y), make_size (x - y));
  }

  TEST_F (MCSMemorySize, operator_plus_works)
  {
    auto random_value {RandomValue{}};
    auto const x {random_value()};
    auto const y {random_value()};

    ASSERT_EQ (make_size (x) + make_size (y), make_size (x + y));
  }

  TEST_F (MCSMemorySize, operator_div_works)
  {
    auto const x {RandomValue{}()};
    auto const y {RandomValue {Min {1u}}()};

    ASSERT_EQ (make_size (x) / make_size (y), x / y);
  }

  TEST_F (MCSMemorySize, operator_mod_works)
  {
    auto const x {RandomValue{}()};
    auto const y {RandomValue {Min {1u}}()};

    ASSERT_EQ (make_size (x) % make_size (y), x % y);
  }

  TEST_F (MCSMemorySize, operator_product_works)
  {
    auto random_value {RandomValue{}};
    auto const x {random_value()};
    auto const y {random_value()};

    ASSERT_EQ (make_size (x) * y, make_size (x * y));
    ASSERT_EQ (y * make_size (x), make_size (y * x));
  }
}

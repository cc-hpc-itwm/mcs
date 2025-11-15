// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/testing/core/printer/memory/Offset.hpp>
#include <mcs/testing/core/random/memory/Offset.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>

namespace
{
  struct ReadOfFmtIsIdentityR : public mcs::testing::random::Test{};
}

TEST_F (ReadOfFmtIsIdentityR, mcs_offset)
{
  mcs::testing::read_of_fmt_is_identity
    (mcs::testing::random::value<mcs::core::memory::Offset>{}());
}

namespace mcs::core::memory
{
  struct MCSMemoryOffset : public testing::random::Test
  {
    using RandomValue = testing::random::value<Offset::underlying_type>;
  };

  TEST_F (MCSMemoryOffset, is_serializable)
  {
    static_assert (serialization::is_serializable<Offset>);

    testing::deserialized_from_serialized_is_identity
      (testing::random::value<Offset>{}());
  }

  TEST_F (MCSMemoryOffset, is_readable_without_prefix)
  {
    auto const value {RandomValue{}()};
    auto const input {fmt::format ("{}", value)};
    ASSERT_EQ (make_offset (value), util::read::read<Offset> (input));
  }

  TEST_F (MCSMemoryOffset, operator_minus_works)
  {
    auto random_value {RandomValue{}};
    auto const x {random_value()};
    auto const y {random_value()};

    ASSERT_EQ (make_offset (x) - make_offset (y), make_size (x - y));
  }

  TEST_F (MCSMemoryOffset, operator_plus_of_offsets_works)
  {
    auto random_value {RandomValue{}};
    auto const x {random_value()};
    auto const y {random_value()};

    ASSERT_EQ (make_offset (x) + make_offset (y), make_offset (x + y));
  }
}

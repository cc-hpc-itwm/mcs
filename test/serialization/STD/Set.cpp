// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/UserDefined.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <mcs/serialization/STD/set.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/bool.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <set>

namespace
{
  template<typename Value, typename Compare = std::less<Value>>
    auto random_set (Compare = {})
  {
    std::set<Value, Compare> s;

    auto random_value {mcs::testing::random::value<Value>{}};

    auto const n {mcs::testing::random::value<decltype (s.size())> {0, 1000}()};

    while (s.size() < n)
    {
      s.emplace (random_value());
    }

    return s;
  }

  struct SerializationR : public mcs::testing::random::Test{};
}

TEST_F (SerializationR, set_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_set<int>());
}
TEST_F (SerializationR, set_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_set<UserDefined>());
}
TEST_F (SerializationR, set_greater_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_set<int, std::greater<>>());
}
TEST_F (SerializationR, set_greater_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_set<UserDefined, std::greater<>>());
}
TEST_F (SerializationR, set_CustomCompare_UserDefined)
{
  auto const compare {CustomCompare {mcs::testing::random::value<bool>{}()}};

  mcs::testing::deserialized_from_serialized_is_identity
    (random_set<UserDefined, CustomCompare> (compare));
}

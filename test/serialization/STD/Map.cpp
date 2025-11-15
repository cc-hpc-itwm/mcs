// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/UserDefined.hpp"
#include <gtest/gtest.h>
#include <map>
#include <mcs/serialization/STD/map.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/bool.hpp>
#include <mcs/testing/random/value/integral.hpp>

namespace
{
  template< typename Key
          , typename Value
          , typename Compare = std::less<Key>
          >
    auto random_map (Compare = {})
  {
    std::map<Key, Value, Compare> m;

    auto random_key {mcs::testing::random::value<Key>{}};
    auto random_value {mcs::testing::random::value<Value>{}};

    auto const n {mcs::testing::random::value<decltype (m.size())> {0, 1000}()};

    while (m.size() < n)
    {
      m.emplace (random_key(), random_value());
    }

    return m;
  }

  struct SerializationR : public mcs::testing::random::Test{};
}

TEST_F (SerializationR, map_int_long)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_map<int, long>());
}
TEST_F (SerializationR, map_int_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_map<int, UserDefined>());
}
TEST_F (SerializationR, map_UserDefined_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_map<UserDefined, int>());
}
TEST_F (SerializationR, map_UserDefined_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_map<UserDefined, UserDefined>());
}
TEST_F (SerializationR, map_CustomCompare_UserDefined_int)
{
  auto const compare {CustomCompare {mcs::testing::random::value<bool>{}()}};

  mcs::testing::deserialized_from_serialized_is_identity
    (random_map<UserDefined, int, CustomCompare> (compare));
}

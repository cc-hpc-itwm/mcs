// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/UserDefined.hpp"
#include <gtest/gtest.h>
#include <mcs/serialization/STD/unordered_map.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/bool.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <unordered_map>

namespace
{
  template< typename Key
          , typename Value
          , typename Hash = std::hash<Key>
          , typename KeyEqual = std::equal_to<Key>
          >
  auto random_unordered_map (Hash hash = {}, KeyEqual key_equal = {})
  {
    std::unordered_map<Key, Value, Hash, KeyEqual> m {0, hash, key_equal};

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

TEST_F (SerializationR, unordered_map_int_long)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_unordered_map<int, long>());
}
TEST_F (SerializationR, unordered_map_int_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_unordered_map<int, UserDefined>());
}
TEST_F (SerializationR, unordered_map_UserDefined_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_unordered_map<UserDefined, int>());
}
TEST_F (SerializationR, unordered_map_UserDefined_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_unordered_map<UserDefined, UserDefined>());
}
TEST_F (SerializationR, unordered_map_CustomHash_UserDefined_int)
{
  auto const hash {CustomHash {mcs::testing::random::value<std::size_t>{}()}};

  mcs::testing::deserialized_from_serialized_is_identity
    (random_unordered_map<UserDefined, int, CustomHash> (hash));
}
TEST_F (SerializationR, unordered_map_CustomHash_CustomEq_UserDefined_int)
{
  auto const hash {CustomHash {mcs::testing::random::value<std::size_t>{}()}};
  auto const eq {CustomEq{}};

  mcs::testing::deserialized_from_serialized_is_identity
    (random_unordered_map<UserDefined, int, CustomHash, CustomEq> (hash, eq));
}

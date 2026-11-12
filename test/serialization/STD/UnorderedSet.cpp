// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/UserDefined.hpp"
#include <gtest/gtest.h>
#include <mcs/serialization/STD/unordered_set.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/bool.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <unordered_map>

namespace
{
  template< typename Key
          , typename Hash = std::hash<Key>
          , typename KeyEqual = std::equal_to<Key>
          >
  auto random_unordered_set (Hash hash = {}, KeyEqual key_equal = {})
  {
    auto s {std::unordered_set<Key, Hash, KeyEqual> {0, hash, key_equal}};

    auto random_key {mcs::testing::random::value<Key>{}};

    auto const n {mcs::testing::random::value<decltype (s.size())> {0, 1000}()};

    while (s.size() < n)
    {
      s.emplace (random_key());
    }

    return s;
  }

  struct SerializationR : public mcs::testing::random::Test{};
}

TEST_F (SerializationR, unordered_set_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_unordered_set<int>());
}
TEST_F (SerializationR, unordered_set_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_unordered_set<UserDefined>());
}
TEST_F (SerializationR, unordered_set_CustomHash_CustomHash)
{
  auto const hash {CustomHash {mcs::testing::random::value<std::size_t>{}()}};

  mcs::testing::deserialized_from_serialized_is_identity
    (random_unordered_set<UserDefined, CustomHash> (hash));
}
TEST_F (SerializationR, unordered_set_CustomHash_CustomEq_UserDefined)
{
  auto const hash {CustomHash {mcs::testing::random::value<std::size_t>{}()}};
  auto const eq {CustomEq{}};

  mcs::testing::deserialized_from_serialized_is_identity
    (random_unordered_set<UserDefined, CustomHash, CustomEq> (hash, eq));
}

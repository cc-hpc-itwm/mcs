// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/UserDefined.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <mcs/serialization/STD/list.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/bool.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <set>

namespace
{
  template<typename Value>
    auto random_list()
  {
    std::list<Value> l;

    auto random_value {mcs::testing::random::value<Value>{}};

    auto const n {mcs::testing::random::value<decltype (l.size())> {0, 1000}()};

    while (l.size() < n)
    {
      l.emplace_back (random_value());
    }

    return l;
  }

  struct SerializationR : public mcs::testing::random::Test{};
}

TEST_F (SerializationR, list_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_list<int>());
}
TEST_F (SerializationR, list_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_list<UserDefined>());
}

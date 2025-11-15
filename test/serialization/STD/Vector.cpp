// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/UserDefined.hpp"
#include <gtest/gtest.h>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <vector>

namespace
{
  template<typename Value>
    auto random_vector()
  {
    std::vector<Value> v;

    auto random_value {mcs::testing::random::value<Value>{}};

    auto const n {mcs::testing::random::value<decltype (v.size())> {0, 1000}()};

    while (v.size() < n)
    {
      v.emplace_back (random_value());
    }

    return v;
  }

  struct SerializationR : public mcs::testing::random::Test{};
}

TEST_F (SerializationR, vector_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_vector<int>());
}
TEST_F (SerializationR, vector_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_vector<UserDefined>());
}

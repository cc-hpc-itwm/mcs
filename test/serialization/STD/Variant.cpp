// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/UserDefined.hpp"
#include <gtest/gtest.h>
#include <mcs/serialization/STD/variant.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <variant>

namespace
{
  template<typename T>
    auto random_single() -> std::variant<T>
  {
    return mcs::testing::random::value<T>{}();
  }

  template<typename T, typename... Ts>
    auto random_multi() -> std::variant<Ts...>
  {
    return mcs::testing::random::value<T>{}();
  }

  struct SerializationR : public mcs::testing::random::Test{};
}

TEST_F (SerializationR, variant_single_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_single<int>());
}
TEST_F (SerializationR, variant_single_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_single<UserDefined>());
}
TEST_F (SerializationR, variant_multi_int_int_long)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_multi<int, int, long>());
}
TEST_F (SerializationR, variant_multi_int_long_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_multi<int, long, int>());
}
TEST_F (SerializationR, variant_multi_int_int_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_multi<int, int, UserDefined>());
}
TEST_F (SerializationR, variant_multi_int_UserDefined_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_multi<int, UserDefined, int>());
}
TEST_F (SerializationR, variant_multi_UserDefined_int_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_multi<UserDefined, int, UserDefined>());
}
TEST_F (SerializationR, variant_multi_UserDefined_UserDefined_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_multi<UserDefined, UserDefined, int>());
}

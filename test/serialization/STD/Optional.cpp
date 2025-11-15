// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/UserDefined.hpp"
#include <gtest/gtest.h>
#include <mcs/serialization/STD/optional.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <optional>

namespace
{
  template<typename Value>
    auto empty_optional() noexcept -> std::optional<Value>
  {
    return {};
  }

  template<typename Value>
    auto random_optional() -> std::optional<Value>
  {
    return mcs::testing::random::value<Value>{}();
  }

  struct SerializationR : public mcs::testing::random::Test{};
}

TEST (Serialization, optional_int_empty)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (empty_optional<int>());
}
TEST (Serialization, optional_UserDefined_empty)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (empty_optional<UserDefined>());
}
TEST_F (SerializationR, optional_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_optional<int>());
}
TEST_F (SerializationR, optional_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_optional<UserDefined>());
}

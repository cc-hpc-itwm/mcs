// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/UserDefined.hpp"
#include <gtest/gtest.h>
#include <mcs/serialization/STD/tuple.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <tuple>

namespace
{
  template<typename...> struct RandomTuple;

  template<> struct RandomTuple<>
  {
    constexpr auto operator()() const noexcept
    {
      return std::make_tuple();
    }
  };

  template<typename T, typename... Ts>
    struct RandomTuple<T, Ts...> : public RandomTuple<Ts...>
  {
    explicit RandomTuple() = default;

    mcs::testing::random::value<T> _value{};

    auto operator()()
    {
      return std::tuple_cat
        ( std::make_tuple (_value())
        , RandomTuple<Ts...>::operator()()
        );
    }
  };

  template<typename... Ts>
    auto random_tuple()
  {
    auto rt {RandomTuple<Ts...>{}};
    return rt();
  }

  struct SerializationR : public mcs::testing::random::Test{};
}

TEST_F (SerializationR, tuple_empty)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_tuple<>());
}
TEST_F (SerializationR, tuple_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_tuple<int>());
}
TEST_F (SerializationR, tuple_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_tuple<UserDefined>());
}
TEST_F (SerializationR, tuple_int_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_tuple<int, int>());
}
TEST_F (SerializationR, tuple_int_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_tuple<int, UserDefined>());
}
TEST_F (SerializationR, tuple_UserDefined_int)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_tuple<UserDefined, int>());
}
TEST_F (SerializationR, tuple_UserDefined_UserDefined)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (random_tuple<UserDefined, UserDefined>());
}

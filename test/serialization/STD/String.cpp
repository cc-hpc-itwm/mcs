// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/serialization/STD/string.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <string>

namespace
{
  struct SerializationR : public mcs::testing::random::Test{};
}

TEST_F (SerializationR, string)
{
  mcs::testing::deserialized_from_serialized_is_identity
    (mcs::testing::random::value<std::string>{}());
}

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/testing/core/operator==/storage/MaxSize.hpp>
#include <mcs/testing/core/printer/storage/MaxSize.hpp>
#include <mcs/testing/core/random/storage/MaxSize.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>

namespace
{
  struct ReadOfFmtIsIdentityR : public mcs::testing::random::Test{};
}

TEST_F (ReadOfFmtIsIdentityR, mcs_storage_max_size)
{
  mcs::testing::read_of_fmt_is_identity
    (mcs::testing::random::value<mcs::core::storage::MaxSize>{}());
}

namespace mcs::core::storage
{
  struct MCSStorageMaxSize : public testing::random::Test{};

  TEST_F (MCSStorageMaxSize, is_serializable)
  {
    static_assert (serialization::is_serializable<MaxSize>);

    testing::deserialized_from_serialized_is_identity
      (testing::random::value<MaxSize>{}());
  }

  TEST_F (MCSStorageMaxSize, no_size_is_larger_than_unlimited)
  {
    auto const unlimited {MaxSize {MaxSize::Unlimited{}}};
    auto const size {testing::random::value<memory::Size>{}()};

    ASSERT_FALSE (size > unlimited);
  }

  TEST_F (MCSStorageMaxSize, relation_greater_propagates_for_limit)
  {
    auto random_size {testing::random::value<memory::Size>{}};
    auto const limit {random_size()};
    auto const size {random_size()};

    ASSERT_EQ ( size > MaxSize {MaxSize::Limit {limit}}
              , size > limit
              );
  }
}

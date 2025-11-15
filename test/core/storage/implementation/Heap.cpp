// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Heap/Parameter/Create.hpp>
#include <mcs/testing/core/printer/storage/implementation/Heap/Parameter/Create.hpp>
#include <mcs/testing/core/random/storage/implementation/Heap/Parameter/Create.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>

namespace mcs::core::storage::implementation
{
  struct MCSStorageHeap : public ::testing::Test{};

  TEST_F (MCSStorageHeap, parameter_create_is_serializable)
  {
    static_assert
      (serialization::is_serializable<typename Heap::Parameter::Create>);

    testing::deserialized_from_serialized_is_identity
      (testing::random::value<typename Heap::Parameter::Create>{}());
  }
  TEST_F (MCSStorageHeap, parameter_create_is_fmt_and_read_able)
  {
    static_assert (fmt::formattable<typename Heap::Parameter::Create>);
    static_assert (util::read::is_readable<typename Heap::Parameter::Create>);

    testing::read_of_fmt_is_identity
      (testing::random::value<typename Heap::Parameter::Create>{}());
  }
}

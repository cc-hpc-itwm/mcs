// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Parameter/Create.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Prefix.hpp>
#include <mcs/testing/core/printer/storage/implementation/Files/Parameter/Create.hpp>
#include <mcs/testing/core/printer/storage/implementation/Files/Prefix.hpp>
#include <mcs/testing/core/random/storage/implementation/Files/Parameter/Create.hpp>
#include <mcs/testing/core/random/storage/implementation/Files/Prefix.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>

namespace mcs::core::storage::implementation
{
  struct MCSStorageFiles : public ::testing::Test{};

  TEST_F (MCSStorageFiles, prefix_is_seriablizable)
  {
    static_assert (serialization::is_serializable<typename Files::Prefix>);

    testing::deserialized_from_serialized_is_identity
      (testing::random::value<typename Files::Prefix>{}());
  }
  TEST_F (MCSStorageFiles, prefix_is_fmt_and_read_able)
  {
    static_assert (fmt::formattable<typename Files::Prefix>);
    static_assert (util::read::is_readable<typename Files::Prefix>);

    testing::read_of_fmt_is_identity
      (testing::random::value<typename Files::Prefix>{}());
  }

  TEST_F (MCSStorageFiles, parameter_create_is_serializable)
  {
    static_assert
      (serialization::is_serializable<typename Files::Parameter::Create>);

    testing::deserialized_from_serialized_is_identity
      (testing::random::value<typename Files::Parameter::Create>{}());
  }
  TEST_F (MCSStorageFiles, parameter_create_is_fmt_and_read_able)
  {
    static_assert (fmt::formattable<typename Files::Parameter::Create>);
    static_assert (util::read::is_readable<typename Files::Parameter::Create>);

    testing::read_of_fmt_is_identity
      (testing::random::value<typename Files::Parameter::Create>{}());
  }
}

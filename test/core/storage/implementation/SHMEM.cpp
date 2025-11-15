// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/testing/core/operator==/storage/implementation/SHMEM/Parameter/Create.hpp>
#include <mcs/testing/core/operator==/storage/implementation/SHMEM/Prefix.hpp>
#include <mcs/testing/core/printer/storage/implementation/SHMEM/Parameter/Create.hpp>
#include <mcs/testing/core/printer/storage/implementation/SHMEM/Prefix.hpp>
#include <mcs/testing/core/random/storage/implementation/SHMEM/Parameter/Create.hpp>
#include <mcs/testing/core/random/storage/implementation/SHMEM/Prefix.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>

namespace mcs::core::storage::implementation
{
  struct MCSStorageSHMEM : public ::testing::Test{};

  TEST_F (MCSStorageSHMEM, prefix_is_seriablizable)
  {
    static_assert (serialization::is_serializable<typename SHMEM::Prefix>);

    testing::deserialized_from_serialized_is_identity
      (testing::random::value<typename SHMEM::Prefix>{}());
  }
  TEST_F (MCSStorageSHMEM, prefix_is_fmt_and_read_able)
  {
    static_assert (fmt::formattable<typename SHMEM::Prefix>);
    static_assert (util::read::is_readable<typename SHMEM::Prefix>);

    testing::read_of_fmt_is_identity
      (testing::random::value<typename SHMEM::Prefix>{}());
  }

  TEST_F (MCSStorageSHMEM, parameter_create_is_serializable)
  {
    static_assert
      (serialization::is_serializable<typename SHMEM::Parameter::Create>);

    testing::deserialized_from_serialized_is_identity
      (testing::random::value<typename SHMEM::Parameter::Create>{}());
  }
  TEST_F (MCSStorageSHMEM, parameter_create_is_fmt_and_read_able)
  {
    static_assert (fmt::formattable<typename SHMEM::Parameter::Create>);
    static_assert (util::read::is_readable<typename SHMEM::Parameter::Create>);

    testing::read_of_fmt_is_identity
      (testing::random::value<typename SHMEM::Parameter::Create>{}());
  }
}

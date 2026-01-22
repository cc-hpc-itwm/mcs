// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/implementation/Virtual.hpp>
#include <mcs/testing/core/operator==/storage/MaxSize.hpp>
#include <mcs/testing/core/printer/storage/MaxSize.hpp>
#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/core/random/storage/MaxSize.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/core/storage/implementation/Virtual.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <stdexcept>
#include <tuple>

namespace mcs::core
{
  namespace
  {
    using SupportedStorageImplementations = util::type::List
      < core::storage::implementation::Files
      , core::storage::implementation::Heap
      , core::storage::implementation::SHMEM
      , core::storage::implementation::Virtual
      >;

    namespace TestingImplementation
      = testing::core::storage::implementation
      ;

    using StoragesT = ::testing::Types
      < TestingImplementation::Files
      , TestingImplementation::Heap
      , TestingImplementation::SHMEM
      , TestingImplementation::Virtual<TestingImplementation::Files>
      , TestingImplementation::Virtual<TestingImplementation::Heap>
      , TestingImplementation::Virtual<TestingImplementation::SHMEM>
      >;
    template<class> struct MCSStorageManagementT
      : public testing::random::Test
    {};
    TYPED_TEST_SUITE (MCSStorageManagementT, StoragesT);
  }

  TYPED_TEST
    ( MCSStorageManagementT
    , storage_create_remembers_max_size_and_uses_zero_bytes
    )
  {
    using TestingStorage = TypeParam;

    auto storages {core::Storages<SupportedStorageImplementations>{}};

    auto const max_size {testing::random::value<storage::MaxSize>{}()};

    auto testing_storage {TestingStorage {max_size}};
    auto const storage
      { make_unique_storage<typename TestingStorage::Storage>
          ( std::addressof (storages)
          , testing_storage.parameter_create()
          )
      };

    auto const read_access {storages.read_access()};

    ASSERT_EQ
      ( max_size
      , read_access.template invoke<typename TestingStorage::Storage>
        ( storage->id()
        , [&] (auto const& storage_implementation)
          {
            return storage_implementation.size_max
              ( testing_storage.parameter_size_max()
              );
          }
        )
      );

    ASSERT_EQ
      ( memory::make_size (0)
      , read_access
        . template invoke<typename TestingStorage::Storage>
          ( storage->id()
          , [&] (auto const& storage_implementation)
            {
              return storage_implementation.size_used
                ( testing_storage.parameter_size_used()
                );
            }
          )
      );
  }

  TYPED_TEST
    ( MCSStorageManagementT
    , access_of_removed_storage_throws
    )
  {
    using TestingStorage = TypeParam;

    auto storages {core::Storages<SupportedStorageImplementations>{}};

    auto testing_storage
      {TestingStorage {testing::random::value<storage::MaxSize>{}()}};
    auto const storage_id
      { make_unique_storage<typename TestingStorage::Storage>
          ( std::addressof (storages)
          , testing_storage.parameter_create()
          )->id() // removed immediately
      };

    testing::require_exception
      ( [&]
        {
          std::ignore = storages.read_access()
            . template invoke<typename TestingStorage::Storage>
              ( storage_id
              , [&] (auto const& storage_implementation)
                {
                  return storage_implementation.size_max
                    ( testing_storage.parameter_size_max()
                    );
                }
              );
        }
      , testing::assert_type_and_what<typename decltype (storages)::Error::UnknownKey>
          (fmt::format ("Unknown key '{}'", storage_id))
      );

    testing::require_exception
      ( [&]
        {
          std::ignore = storages.read_access()
            . template invoke<typename TestingStorage::Storage>
              ( storage_id
              , [&] (auto const& storage_implementation)
                {
                  return storage_implementation.size_used
                    ( testing_storage.parameter_size_used()
                    );
                }
              );
        }
      , testing::assert_type_and_what<typename decltype (storages)::Error::UnknownKey>
          (fmt::format ("Unknown key '{}'", storage_id))
      );

    testing::require_exception
      ( [&]
        {
          std::ignore = storages.read_write_access()
            . template modify<typename TestingStorage::Storage>
              ( storage_id
              , [&] (auto& storage_implementation)
                {
                  return storage_implementation.segment_create
                    ( testing_storage.parameter_segment_create()
                    , testing::random::value<memory::Size>{}()
                    );
                }
              );
        }
      , testing::assert_type_and_what<typename decltype (storages)::Error::UnknownKey>
          (fmt::format ("Unknown key '{}'", storage_id))
      );
  }
}

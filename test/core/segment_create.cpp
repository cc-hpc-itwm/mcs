// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <gtest/gtest.h>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/testing/core/operator==/storage/MaxSize.hpp>
#include <mcs/testing/core/printer/memory/Size.hpp>
#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/core/storage/implementation/Virtual.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/random_device.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace mcs::core
{
  namespace
  {
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
    template<class> struct MCSSegmentCreateT : public testing::random::Test{};
    TYPED_TEST_SUITE (MCSSegmentCreateT, StoragesT);

    using SupportedStorageImplementations = util::type::List
      < core::storage::implementation::Files
      , core::storage::implementation::Heap
      , core::storage::implementation::SHMEM
      , core::storage::implementation::Virtual
      >;
  }

  TYPED_TEST
    ( MCSSegmentCreateT
    , segment_create_unlimited_correctly_counts_size_used
    )
  {
    using TestingStorage = TypeParam;

    auto storages {core::Storages<SupportedStorageImplementations>{}};

    auto testing_storage
      {TestingStorage {storage::MaxSize {storage::MaxSize::Unlimited{}}}};
    auto const storage
      { make_unique_storage<typename TestingStorage::Storage>
          ( std::addressof (storages)
          , testing_storage.parameter_create()
          )
      };

    ASSERT_FALSE
      ( testing::random::value<memory::Size>{}()
      > storages.template size_max<typename TestingStorage::Storage>
        ( storages.read_access()
        , storage->id()
        , testing_storage.parameter_size_max()
        )
      );

    auto size_used {memory::make_size (0)};

    ASSERT_EQ
      ( storages.template size_used<typename TestingStorage::Storage>
          ( storages.read_access()
          , storage->id()
          , testing_storage.parameter_size_used()
          )
      , size_used
      );

    struct SegmentIDWithSize
    {
      storage::segment::ID _segment_id;
      memory::Size _size;
    };

    auto segment_ids_with_sizes {std::vector<SegmentIDWithSize>{}};
    auto random_size {testing::random::value<memory::Size> {1 << 10, 1 << 20}};

    auto number_of_segments {testing::random::value<int> {0, 100}()};

    while (number_of_segments --> 0)
    {
      auto const size {random_size()};

      segment_ids_with_sizes.emplace_back
        ( storages.template segment_create<typename TestingStorage::Storage>
          ( storages.write_access()
          , storage->id()
          , testing_storage.parameter_segment_create()
          , size
          )
        , size
        );

      size_used += size;

      ASSERT_EQ
        ( storages.template size_used<typename TestingStorage::Storage>
          ( storages.read_access()
          , storage->id()
          , testing_storage.parameter_size_used()
          )
        , size_used
        );
    }

    std::ranges::shuffle
      ( segment_ids_with_sizes
      , testing::random::random_device()
      );

    for (auto [segment_id, size] : segment_ids_with_sizes)
    {
      auto const size_freed
        { storages.template segment_remove<typename TestingStorage::Storage>
          ( storages.write_access()
          , storage->id()
          , testing_storage.parameter_segment_remove()
          , segment_id
          )
        };

      ASSERT_EQ (size_freed, size);

      size_used -= size;

      ASSERT_EQ
        ( storages.template size_used<typename TestingStorage::Storage>
          ( storages.read_access()
          , storage->id()
          , testing_storage.parameter_size_used()
          )
        , size_used
        );
    }

    ASSERT_EQ (size_used, memory::make_size (0));

    ASSERT_EQ
      ( storages.template size_used<typename TestingStorage::Storage>
          ( storages.read_access()
          , storage->id()
          , testing_storage.parameter_size_used()
          )
      , size_used
      );
  }

  TYPED_TEST
    ( MCSSegmentCreateT
    , segment_create_limited_respects_limit
    )
  {
    using TestingStorage = TypeParam;

    auto storages {core::Storages<SupportedStorageImplementations>{}};

    auto random_size {testing::random::value<memory::Size> {1 << 20, 4 << 20}};
    auto const max_size {random_size()};

    auto testing_storage
      {TestingStorage {storage::MaxSize {storage::MaxSize::Limit {max_size}}}};
    auto const storage
      { make_unique_storage<typename TestingStorage::Storage>
          ( std::addressof (storages)
          , testing_storage.parameter_create()
          )
      };

    auto size_used {memory::make_size (0)};

    while (! (size_used > max_size))
    {
      auto const size {random_size()};

      auto create_segment
        { [&]
          {
            std::ignore = storages.segment_create<typename TestingStorage::Storage>
              ( storages.write_access()
              , storage->id()
              , testing_storage.parameter_segment_create()
              , size
              );
          }
        };

      if (size_used + size > max_size)
      {
        testing::require_exception
          ( create_segment
          , testing::Assert<typename TestingStorage::Storage::Error::BadAlloc>
            { [&] (auto const& caught)
              {
                ASSERT_EQ (caught.requested(), size);
                ASSERT_EQ (caught.used(), size_used);
                ASSERT_EQ (caught.max(), storage::MaxSize::Limit {max_size});
              }
            }
          );
      }
      else
      {
        create_segment();
      }

      size_used += size;
    }
  }

  TYPED_TEST
    ( MCSSegmentCreateT
    , zero_size_segment_can_be_created_and_removed
    )
  {
    using TestingStorage = TypeParam;

    auto storages {core::Storages<SupportedStorageImplementations>{}};

    auto testing_storage
      {TestingStorage {storage::MaxSize {storage::MaxSize::Unlimited{}}}};
    auto const storage
      { make_unique_storage<typename TestingStorage::Storage>
          ( std::addressof (storages)
          , testing_storage.parameter_create()
          )
      };

    auto const segment_id
      { storages.segment_create<typename TestingStorage::Storage>
        ( storages.write_access()
        , storage->id()
        , testing_storage.parameter_segment_create()
        , memory::make_size (0)
        )
      };

    ASSERT_EQ
      ( storages.segment_remove<typename TestingStorage::Storage>
        ( storages.write_access()
        , storage->id()
        , testing_storage.parameter_segment_remove()
        , segment_id
        )
      , memory::make_size (0)
      );
  }
}

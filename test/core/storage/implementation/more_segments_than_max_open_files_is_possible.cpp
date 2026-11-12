// Copyright (C) 2024-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <exception>
#include <functional>
#include <gtest/gtest.h>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/random_device.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/syscall/getrlimit.hpp>
#include <mcs/testing/syscall/setrlimit.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read_file.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace mcs::core
{
  namespace
  {
    template<class> struct MCSStorageMoreSegmentsThanMaxOpenFiles
      : public testing::random::Test
    {};

    using SupportedStorageImplementations = util::type::List
      < core::storage::implementation::Files
      , core::storage::implementation::Heap
      , core::storage::implementation::SHMEM
      >;

    using TestStorages = ::testing::Types
      < testing::core::storage::implementation::Files
      , testing::core::storage::implementation::Heap
      , testing::core::storage::implementation::SHMEM
      >;
    TYPED_TEST_SUITE (MCSStorageMoreSegmentsThanMaxOpenFiles, TestStorages);
  }

  TYPED_TEST (MCSStorageMoreSegmentsThanMaxOpenFiles, works)
  {
    using StorageImplementation = TypeParam;

    auto const max_open_files
      { std::invoke
        ( []
          {
            auto const max_map_count
              { std::invoke
                ( []
                  {
                    auto const content
                      { util::read_file ("/proc/sys/vm/max_map_count")
                      };
                    auto state {util::read::make_state (content)};

                    return util::read::parse<unsigned long> (state);
                  }
                )
              };

            auto const _max_open_files
              { std::min
                ( std::min
                  ( 512UL
                  , util::cast<unsigned long>
                    ( testing::syscall::getrlimit (RLIMIT_NOFILE).rlim_cur
                    )
                  )
                , max_map_count / 4UL
                  //            ^ The test will create max_open_files
                  // * 3 many segments, each does a mmap. Limit to
                  // max_map_count / 4 to leave some space.
                )
              };

            testing::syscall::setrlimit
              ( RLIMIT_NOFILE
              , rlimit { .rlim_cur = _max_open_files
                       , .rlim_max = _max_open_files
                       }
              );

            return _max_open_files;
          }
        )
      };

    auto storages {core::Storages<SupportedStorageImplementations>{}};

    auto const storage_implementation {StorageImplementation{}};
    auto const storage
      { make_unique_storage<typename StorageImplementation::Storage>
          ( std::addressof (storages)
          , storage_implementation.parameter_create()
          )
      };

    using Element = std::remove_const_t<decltype (max_open_files)>;

    struct Segment
    {
      SupportedStorageImplementations::template wrap
        < storage::UniqueSegment
        , typename StorageImplementation::Storage
        > _unique_segment;
      memory::Size size;
      Element n;
      storage::segment::ID id {_unique_segment->id()};
    };

    auto random_number_of_elements
      {testing::random::value<memory::Size> {1 << 0, 1 << 10}};

    auto segments {std::vector<Segment>{}};

    auto const number_of_segments
      { testing::random::value<decltype (max_open_files)>
          { max_open_files + 1
          , max_open_files + 2 * max_open_files
          }()
      };

    // Create and fill more than max_number_of_open_files many segments
    //
    for (auto n {decltype (max_open_files) {0}}; n != number_of_segments; ++n)
    {
      auto size {random_number_of_elements() * sizeof (Element)};

      auto const& segment
        { segments.emplace_back
          ( storage::make_unique_segment
              < typename StorageImplementation::Storage
              >
            ( std::addressof (storages)
            , storage->id()
            , size
            , storage_implementation.parameter_segment_create()
            , storage_implementation.parameter_segment_remove()
            )
          , size
          , n
          )
        };

      auto const chunk
        { SupportedStorageImplementations::template wrap
              < Chunk
              , chunk::access::Mutable
              >
          { storages.read_access()
            . template invoke<typename StorageImplementation::Storage>
              ( storage->id()
              , [&] (auto const& _storage_implementation)
                {
                  return _storage_implementation
                    . template chunk_description<chunk::access::Mutable>
                      ( storage_implementation.parameter_chunk_description()
                      , segment.id
                      , memory::make_range
                        ( memory::make_offset (0)
                        , segment.size
                        )
                      )
                    ;
                }
              )
          }
        };
      auto const elements {as<Element> (chunk)};
      std::iota (std::begin (elements), std::end (elements), n);
    }

    // all segments can be used (in any order)
    //
    auto order {std::vector<std::size_t> (segments.size())};
    std::iota (std::begin (order), std::end (order), 0);
    std::ranges::shuffle (order, testing::random::random_device());

    for (auto i : order)
    {
      auto const& segment {segments.at (i)};
      auto const chunk
        { SupportedStorageImplementations::template wrap
              < Chunk
              , chunk::access::Const
              >
          { storages.read_access()
            . template invoke<typename StorageImplementation::Storage>
              ( storage->id()
              , [&] (auto const& _storage_implementation)
                {
                  return _storage_implementation
                    . template chunk_description<chunk::access::Const>
                      ( storage_implementation.parameter_chunk_description()
                      , segment.id
                      , memory::make_range
                        ( memory::make_offset (0)
                        , segment.size
                        )
                      )
                    ;
                }
              )
          }
        };
      auto const elements {as<Element const> (chunk)};
      ASSERT_EQ ( memory::make_size (elements.size() * sizeof (Element))
                , segment.size
                );

      auto expected {segment.n};

      for (auto element : elements)
      {
        ASSERT_EQ (element, expected++);
      }
    }
  }
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gtest/gtest.h>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/core/storage/implementation/Trace.hpp>
#include <mcs/core/storage/trace/Events.hpp>
#include <mcs/core/storage/tracer/LogFile.hpp>
#include <mcs/core/storage/tracer/Record.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Chunk/Description.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Parameter.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Heap/Chunk/Description.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Heap/Parameter.hpp>
#include <mcs/testing/core/operator==/storage/implementation/SHMEM/Chunk/Description.hpp>
#include <mcs/testing/core/operator==/storage/implementation/SHMEM/Parameter.hpp>
#include <mcs/testing/core/operator==/storage/trace/Events.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/FMT/STD/variant.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/read_file.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <tuple>
#include <vector>

namespace mcs::core
{
  namespace
  {
    using Files = storage::implementation::Files;
    using Heap = storage::implementation::Heap;
    using SHMEM = storage::implementation::SHMEM;

    using LogFileFiles
      = storage::implementation::Trace<storage::tracer::LogFile<Files>, Files>
      ;
    using LogFileHeap
      = storage::implementation::Trace<storage::tracer::LogFile<Heap>, Heap>
      ;
    using LogFileSHMEM
      = storage::implementation::Trace<storage::tracer::LogFile<SHMEM>, SHMEM>
      ;

    using RecordFiles
      = storage::implementation::Trace<storage::tracer::Record<Files>, Files>
      ;
    using RecordHeap
      = storage::implementation::Trace<storage::tracer::Record<Heap>, Heap>
      ;
    using RecordSHMEM
      = storage::implementation::Trace<storage::tracer::Record<SHMEM>, SHMEM>
      ;

    using SupportedStorageImplementations = util::type::List
      < LogFileFiles
      , LogFileHeap
      , LogFileSHMEM
      , RecordFiles
      , RecordHeap
      , RecordSHMEM
      >;
  }

  namespace
  {
    template< typename TestingStorage
            , template<typename> typename Tracer
            >
    struct TracedStorage
    {
      using Storage = typename TestingStorage::Storage;
      using TracedStorageImplementation
        = storage::implementation::Trace<Tracer<Storage>, Storage>
        ;

      static_assert
        ( SupportedStorageImplementations
          ::template contains<TracedStorageImplementation>()
        );

      template<typename Tag>
        TracedStorage
          ( util::not_null<Storages<SupportedStorageImplementations>> storages
          , Tag tag
          , memory::Size size
          , typename Tracer<Storage>::Parameter::Create tracer_parameter_create
          )
            : _storages {storages}
            , _size {size}
            , _tracer_parameter_create {tracer_parameter_create}
            , _testing_storage {tag}
      {}

      [[nodiscard]] constexpr auto parameter_create() const noexcept
      {
        return _testing_storage.parameter_create();
      }
      [[nodiscard]] constexpr auto size() const noexcept
      {
        return _size;
      }
      [[nodiscard]] constexpr auto range() const noexcept
      {
        return _range;
      }
      [[nodiscard]] constexpr auto segment_id() const noexcept
      {
        return _segment->id();
      }
      [[nodiscard]] constexpr auto chunk() const noexcept
        -> SupportedStorageImplementations::template wrap
            < Chunk
            , chunk::access::Mutable
            > const&
      {
        return _chunk;
      }

      template<typename ChunkDescription>
        [[nodiscard]] auto expected_events
          ( ChunkDescription chunk_description
          ) const
      {
        auto events
          { std::vector<typename storage::trace::Events<Storage>::Variant>{}
          };

        events.emplace_back
          ( storage::trace::event::Create<Storage>
            { _testing_storage.parameter_create()
            }
          );
        events.emplace_back
          ( storage::trace::event::segment::Create<Storage>
            { _testing_storage.parameter_segment_create()
            , _size
            }
          );
        events.emplace_back
          ( storage::trace::event::segment::create::Result
            { _segment->id()
            }
          );
        events.emplace_back
          ( storage::trace::event::chunk::Description
              < Storage
              , chunk::access::Mutable
              >
            { _testing_storage.parameter_chunk_description()
            , _segment->id()
            , core::memory::make_range
              ( core::memory::make_offset (0)
              , _size
              )
            }
          );
        events.emplace_back
          ( storage::trace::event::chunk::description::Result
              < Storage
              , chunk::access::Mutable
              >
            { chunk_description
            }
          );
        events.emplace_back
          ( storage::trace::event::segment::Remove<Storage>
            { _testing_storage.parameter_segment_remove()
            , _segment->id()
            }
          );
        events.emplace_back
          ( storage::trace::event::segment::remove::Result
            { _size
            }
          );
        events.emplace_back
          ( storage::trace::event::Destruct{}
          );

        return events;
      }


      template<typename ChunkDescription>
        [[nodiscard]] auto expected_log
          ( ChunkDescription chunk_description
          ) const
      {
        return fmt::format
          ( "{}\n"
          , fmt::join (expected_events (chunk_description), "\n")
          );
      }

    private:
      util::not_null<Storages<SupportedStorageImplementations>> _storages;
      memory::Size _size;
      typename Tracer<Storage>::Parameter::Create _tracer_parameter_create;
      TestingStorage _testing_storage;

      memory::Range _range
        { memory::make_range (memory::make_offset (0), _size)
        };

      SupportedStorageImplementations::template wrap
        < UniqueStorage
        , TracedStorageImplementation
        > _storage
          { make_unique_storage<TracedStorageImplementation>
              ( _storages
              , typename TracedStorageImplementation::Parameter::Create
                { _tracer_parameter_create
                , _testing_storage.parameter_create()
                }
              )
          };
      SupportedStorageImplementations::template wrap
        < storage::UniqueSegment
        , TracedStorageImplementation
        > _segment
          { storage::make_unique_segment<TracedStorageImplementation>
              ( _storages
              , _storage->id()
              , _size
              , _testing_storage.parameter_segment_create()
              , _testing_storage.parameter_segment_remove()
              )
          };

      SupportedStorageImplementations::template wrap
        < Chunk
        , chunk::access::Mutable
        > _chunk
          { _storages->template chunk_description
              < TracedStorageImplementation
              , chunk::access::Mutable
              >
              ( _storages->read_access()
              , _storage->id()
              , _testing_storage.parameter_chunk_description()
              , _segment->id()
              , _range
              )
          };
    };
  }

  namespace
  {
    struct MCSStorageTracingR : public testing::random::Test{};
  }

  // Create some traced memories in one container, use them and check
  // their traces.
  //
  TEST_F (MCSStorageTracingR, storage_operations_can_be_traced)
  {
    auto const working_directory
      { testing::UniqTemporaryDirectory {"MCSStorageTracing"}
      };
    auto random_memory_size
      {testing::random::value<std::size_t> {2 << 10, 2 << 20}};


    // Check the events after the storages with their tracers have been
    // destroyed and the ofstream has been flushed and the destruction
    // events have been produced.
    auto record_files {typename storage::tracer::Record<Files>::Events{}};
    auto record_heap {typename storage::tracer::Record<Heap>::Events{}};
    auto record_shmem {typename storage::tracer::Record<SHMEM>::Events{}};

    auto const [ expected_events_files
               , expected_events_heap
               , expected_events_shmem
               , expected_log_file_files
               , expected_log_file_heap
               , expected_log_file_shmem
               ]
    { std::invoke
      ( [&]
        {
          auto storages {Storages<SupportedStorageImplementations>{}};

          auto const traced_storage_log_file_files
            { TracedStorage< testing::core::storage::implementation::Files
                           , storage::tracer::LogFile
                           >
              { std::addressof (storages)
              , "log_file_files"
              , memory::make_size (random_memory_size())
              , typename storage::tracer::LogFile<Files>::Parameter::Create
                { working_directory.path() / "log_file_files"
                }
              }
            };
          auto const traced_storage_log_file_heap
            { TracedStorage< testing::core::storage::implementation::Heap
                           , storage::tracer::LogFile
                           >
              { std::addressof (storages)
              , "log_file_heap"
              , memory::make_size (random_memory_size())
              , typename storage::tracer::LogFile<Heap>::Parameter::Create
                { working_directory.path() / "log_file_heap"
                }
              }
            };
          auto const traced_storage_log_file_shmem
            { TracedStorage< testing::core::storage::implementation::SHMEM
                           , storage::tracer::LogFile
                           >
              { std::addressof (storages)
              , "log_file_shmem"
              , memory::make_size (random_memory_size())
              , typename storage::tracer::LogFile<SHMEM>::Parameter::Create
                { working_directory.path() / "log_file_shmem"
                }
              }
            };

          auto const traced_storage_record_files
            { TracedStorage< testing::core::storage::implementation::Files
                           , storage::tracer::Record
                           >
              { std::addressof (storages)
              , "record_files"
              , memory::make_size (random_memory_size())
              , typename storage::tracer::Record<Files>::Parameter::Create
                { std::addressof (record_files)
                }
              }
            };
          auto const traced_storage_record_heap
            { TracedStorage< testing::core::storage::implementation::Heap
                           , storage::tracer::Record
                           >
              { std::addressof (storages)
              , "record_heap"
              , memory::make_size (random_memory_size())
              , typename storage::tracer::Record<Heap>::Parameter::Create
                { std::addressof (record_heap)
                }
              }
            };
          auto const traced_storage_record_shmem
            { TracedStorage< testing::core::storage::implementation::SHMEM
                           , storage::tracer::Record
                           >
              { std::addressof (storages)
              , "record_shmem"
              , memory::make_size (random_memory_size())
              , typename storage::tracer::Record<SHMEM>::Parameter::Create
                { std::addressof (record_shmem)
                }
              }
            };

          auto const make_chunk_description_files
            { [] (auto const& files_storage)
              {
                using Mutable = core::chunk::access::Mutable;

                return Files::Chunk::Description<Mutable>
                  { files_storage.parameter_create().prefix.value
                    / fmt::format ("{}", files_storage.segment_id())
                  , files_storage.size()
                  , files_storage.range()
                  };
              }
            };
          auto const make_chunk_description_heap
            { [] (auto const& heap_storage)
              {
                using Mutable = core::chunk::access::Mutable;

                return Heap::Chunk::Description<Mutable>
                  { util::cast<std::uintmax_t>
                    ( heap_storage.chunk().data().data()
                    )
                  , core::memory::size_cast<std::size_t>
                    ( heap_storage.size()
                    )
                  , heap_storage.range()
                  };
              }
            };
          auto const make_chunk_description_shmem
            { [] (auto const& shmem_storage)
              {
                using Mutable = core::chunk::access::Mutable;

                return SHMEM::Chunk::Description<Mutable>
                  { shmem_storage.parameter_create().prefix
                  , shmem_storage.segment_id()
                  , shmem_storage.size()
                  , shmem_storage.range()
                  };
              }
            };

          return std::make_tuple
            ( traced_storage_record_files.expected_events
              ( make_chunk_description_files (traced_storage_record_files)
              )
            , traced_storage_record_heap.expected_events
              ( make_chunk_description_heap (traced_storage_record_heap)
              )
            , traced_storage_record_shmem.expected_events
              ( make_chunk_description_shmem (traced_storage_record_shmem)
              )

            , traced_storage_log_file_files.expected_log
              ( make_chunk_description_files (traced_storage_log_file_files)
              )
            , traced_storage_log_file_heap.expected_log
              ( make_chunk_description_heap (traced_storage_log_file_heap)
              )
            , traced_storage_log_file_shmem.expected_log
              ( make_chunk_description_shmem (traced_storage_log_file_shmem)
              )
            );
        }
      )
    };

    EXPECT_EQ (expected_events_files, record_files);
    EXPECT_EQ (expected_events_heap, record_heap);
    EXPECT_EQ (expected_events_shmem, record_shmem);

    EXPECT_EQ
      ( expected_log_file_files
      , util::read_file (working_directory.path() / "log_file_files")
      );
    EXPECT_EQ
      ( expected_log_file_heap
      , util::read_file (working_directory.path() / "log_file_heap")
      );
    EXPECT_EQ
      ( expected_log_file_shmem
      , util::read_file (working_directory.path() / "log_file_shmem")
      );
  }
}

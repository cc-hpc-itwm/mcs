// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/core/storage/implementation/Virtual.hpp>
#include <mcs/core/storage/select.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/read/ParseError.hpp>
#include <mcs/util/type/List.hpp>
#include <type_traits>

namespace
{
  namespace selection = mcs::core::storage::selection;

  using SupportedStorageImplementations = mcs::util::type::List
    < mcs::core::storage::implementation::Files
    , mcs::core::storage::implementation::Heap
    , mcs::core::storage::implementation::SHMEM
    , mcs::core::storage::implementation::Virtual
    >;

  template<typename Expected>
    auto selects (selection::Arguments const& arguments) -> bool
  {
    return selection::select<SupportedStorageImplementations>
      ( arguments
      , []<mcs::core::storage::is_implementation Implementation>
          (selection::Parameters<Implementation>)
        {
          return std::is_same_v<Implementation, Expected>;
        }
      );
  }

  TEST (CoreStorageSelection, selects_heap)
  {
    EXPECT_TRUE
      ( selects<mcs::core::storage::implementation::Heap>
        ( { "Heap (Limit 1048576)"
          , "Heap::Size::Max()"
          , "Heap::Size::Used()"
          , "Heap::Segment::Create (Nothing)"
          , "Heap::Segment::Remove()"
          , "Heap::Chunk::Description()"
          }
        )
      );
  }

  TEST (CoreStorageSelection, selects_files)
  {
    auto const temporary
      { mcs::testing::UniqTemporaryDirectory {"StorageSelectionFiles"}
      };

    EXPECT_TRUE
      ( selects<mcs::core::storage::implementation::Files>
        ( { fmt::format
              ( "Files (Prefix \"{}\", Limit 1048576)"
              , temporary.path().string()
              )
          , "Files::Size::Max()"
          , "Files::Size::Used()"
          , "Files::Segment::Create"
              " (Files::Segment::OnRemove::Remove())"
          , "Files::Segment::Remove (Nothing)"
          , "Files::Chunk::Description()"
          }
        )
      );
  }

  TEST (CoreStorageSelection, selects_shared_memory)
  {
    EXPECT_TRUE
      ( selects<mcs::core::storage::implementation::SHMEM>
        ( { "SHMEM (Prefix \"StorageSelection\", Limit 1048576)"
          , "SHMEM::Size::Max()"
          , "SHMEM::Size::Used()"
          , "SHMEM::Segment::Create"
              " (SHMEM::Segment::AccessMode (384), Nothing)"
          , "SHMEM::Segment::Remove()"
          , "SHMEM::Chunk::Description()"
          }
        )
      );
  }

  TEST (CoreStorageSelection, selects_virtual)
  {
    using Virtual = mcs::core::storage::implementation::Virtual;
    using Parameter = Virtual::Parameter;

    auto const bytes {Virtual::StorageParameter {1U, 2U, 3U}};

    EXPECT_TRUE
      ( selects<Virtual>
        ( { fmt::format
              ( "{}"
              , Parameter::Create {"/tmp/mcs-virtual.so", bytes}
              )
          , fmt::format ("{}", Parameter::Size::Max {bytes})
          , fmt::format ("{}", Parameter::Size::Used {bytes})
          , fmt::format ("{}", Parameter::Segment::Create {bytes})
          , fmt::format ("{}", Parameter::Segment::Remove {bytes})
          , fmt::format ("{}", Parameter::Chunk::Description {bytes})
          }
        )
      );
  }

  TEST (CoreStorageSelection, supported_implementations_are_caller_selected)
  {
    using HeapOnly = mcs::util::type::List
      < mcs::core::storage::implementation::Heap
      >;

    mcs::testing::require_exception
      ( []
        {
          selection::select<HeapOnly>
            ( { .storage = "Files (Prefix \"/tmp\", Limit 1048576)"
              , .parameter_size_max = ""
              , .parameter_size_used = ""
              , .parameter_segment_create = ""
              , .parameter_segment_remove = ""
              , .parameter_chunk_description = ""
              }
            , []<mcs::core::storage::is_implementation Implementation>
                (selection::Parameters<Implementation>)
              {}
            );
        }
      , mcs::testing::assert_type<mcs::util::read::ParseError>()
      );
  }

  TEST (CoreStorageSelection, rejects_unknown_storage)
  {
    mcs::testing::require_exception
      ( []
        {
          selection::select<SupportedStorageImplementations>
            ( { .storage = "Unknown()"
              , .parameter_size_max = ""
              , .parameter_size_used = ""
              , .parameter_segment_create = ""
              , .parameter_segment_remove = ""
              , .parameter_chunk_description = ""
              }
            , []<mcs::core::storage::is_implementation Implementation>
                (selection::Parameters<Implementation>) {}
            );
        }
      , mcs::testing::assert_type<mcs::util::read::ParseError>()
      );
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <array>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/core/storage/implementation/Virtual.hpp>
#include <mcs/fuse/content/implementation/Realloc.hpp>
#include <mcs/fuse/content/implementation/Storage.hpp>
#include <mcs/fuse/content/implementation/Vector.hpp>
#include <mcs/fuse/content/select.hpp>
#include <mcs/fuse/option.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/Virtual.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/option/Parameter.hpp>
#include <mcs/util/read/ParseError.hpp>
#include <mcs/util/type/List.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace
{
  using SupportedStorageImplementations = mcs::util::type::List
    < mcs::core::storage::implementation::Files
    , mcs::core::storage::implementation::Heap
    , mcs::core::storage::implementation::SHMEM
    , mcs::core::storage::implementation::Virtual
    >;

  template<typename Expected>
    auto selects
      ( std::vector<std::string> arguments
      , std::vector<std::string> const& expected_remaining = {}
      ) -> bool
  {
    auto pointers {std::vector<char const*>{}};
    pointers.reserve (arguments.size());

    std::ranges::transform
      ( arguments
      , std::back_inserter (pointers)
      , [] (auto const& argument) noexcept
        {
          return argument.c_str();
        }
      );

    return mcs::fuse::content::selection::select
      < SupportedStorageImplementations
      >
      ( mcs::util::arguments::View {pointers.data(), pointers.size()}
      , [&]<mcs::fuse::is_content Content>
          ( typename Content::State
          , mcs::util::arguments::View remaining
          )
        {
          if constexpr (!std::is_same_v<Content, Expected>)
          {
            return false;
          }

          return std::ranges::equal (remaining, expected_remaining);
        }
      );
  }

  TEST (FuseContentSelection, named_selection_rejects_storage_options)
  {
    std::ranges::for_each
      ( std::array
        { mcs::fuse::option::storage
        , mcs::fuse::option::storage_size_max
        , mcs::fuse::option::storage_size_used
        , mcs::fuse::option::storage_segment_create
        , mcs::fuse::option::storage_segment_remove
        , mcs::fuse::option::storage_chunk_description
        }
      , [] (mcs::util::command_line::option::Parameter const& name)
        {
          auto const option {fmt::format ("--{}=value", name)};
          auto const pointers
            { std::array {"--content=Vector", option.c_str()}
            };
          auto const arguments
            { mcs::util::arguments::View {pointers.data(), pointers.size()}
            };
          auto const parsed
            { arguments.parse
              ( mcs::fuse::content::selection::options::schema()
              )
            };
          mcs::testing::require_exception
            ( [&]
              {
                std::ignore = mcs::fuse::content::selection::Arguments {parsed};
              }
            , mcs::testing::assert_type_and_what<std::invalid_argument>
                ("storage options require --content Storage")
            );
        }
      );
  }

  TEST (FuseContentSelection, named_arguments_have_observers)
  {
    static_assert
      (!std::is_aggregate_v<mcs::fuse::content::selection::Arguments>);
    auto const pointers {std::array {"--content=Vector"}};
    auto const arguments
      {mcs::util::arguments::View {pointers.data(), pointers.size()}};
    auto const parsed
      { arguments.parse (mcs::fuse::content::selection::options::schema())
      };
    auto const selection {mcs::fuse::content::selection::Arguments {parsed}};

    EXPECT_EQ (selection.content(), "Vector");
    EXPECT_FALSE (selection.storage().has_value());
  }

  TEST (FuseContentSelection, selects_vector_and_preserves_arguments)
  {
    EXPECT_TRUE
      ( selects<mcs::fuse::content::implementation::Vector>
        ( {"Vector", "name", "mountpoint"}
        , {"name", "mountpoint"}
        )
      );
  }

  TEST (FuseContentSelection, selects_realloc)
  {
    EXPECT_TRUE
      ( selects<mcs::fuse::content::implementation::Realloc>
        ( {"Realloc"}
        )
      );
  }

  TEST (FuseContentSelection, selects_heap_storage)
  {
    using Content = mcs::fuse::content::implementation::Storage
      < mcs::core::storage::implementation::Heap
      >;

    EXPECT_TRUE
      ( selects<Content>
        ( { "Storage"
          , "Heap (Limit 1048576)"
          , "Heap::Size::Max()"
          , "Heap::Size::Used()"
          , "Heap::Segment::Create (Nothing)"
          , "Heap::Segment::Remove()"
          , "Heap::Chunk::Description()"
          }
        )
      );
  }

  TEST (FuseContentSelection, selects_files_storage)
  {
    auto temporary
      { mcs::testing::UniqTemporaryDirectory {"FuseContentSelectionFiles"}
      };
    using Content = mcs::fuse::content::implementation::Storage
      < mcs::core::storage::implementation::Files
      >;

    EXPECT_TRUE
      ( selects<Content>
        ( { "Storage"
          , fmt::format
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

  TEST (FuseContentSelection, selects_shared_memory_storage)
  {
    using Content = mcs::fuse::content::implementation::Storage
      < mcs::core::storage::implementation::SHMEM
      >;

    EXPECT_TRUE
      ( selects<Content>
        ( { "Storage"
          , "SHMEM (Prefix \"FuseContentSelection\", Limit 1048576)"
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

  TEST (FuseContentSelection, selects_virtual_storage)
  {
    using Virtual = mcs::core::storage::implementation::Virtual;
    using Content = mcs::fuse::content::implementation::Storage<Virtual>;
    using TestingHeap = mcs::testing::core::storage::implementation::Heap;
    using TestingVirtual
      = mcs::testing::core::storage::implementation::Virtual<TestingHeap>
      ;
    auto const storage
      { TestingVirtual
        { mcs::core::storage::MaxSize::Limit
          { mcs::core::memory::make_size (1048576UL)
          }
        }
      };

    EXPECT_TRUE
      ( selects<Content>
        ( { "Storage"
          , fmt::format ("{}", storage.parameter_create())
          , fmt::format ("{}", storage.parameter_size_max())
          , fmt::format ("{}", storage.parameter_size_used())
          , fmt::format ("{}", storage.parameter_segment_create())
          , fmt::format ("{}", storage.parameter_segment_remove())
          , fmt::format ("{}", storage.parameter_chunk_description())
          }
        )
      );
  }

  TEST (FuseContentSelection, rejects_missing_content)
  {
    auto arguments {std::vector<char const*>{}};

    mcs::testing::require_exception
      ( [&]
        {
          mcs::fuse::content::selection::select
            < SupportedStorageImplementations
            >
            ( mcs::util::arguments::View {arguments.data(), arguments.size()}
            , []<mcs::fuse::is_content Content>
                ( typename Content::State
                , mcs::util::arguments::View
                )
              {}
            );
        }
      , mcs::testing::assert_type<std::invalid_argument>()
      );
  }

  TEST (FuseContentSelection, rejects_unknown_content)
  {
    mcs::testing::require_exception
      ( []
        {
          selects<mcs::fuse::content::implementation::Vector>
            ( {"Unknown"}
            );
        }
      , mcs::testing::assert_type<std::invalid_argument>()
      );
  }

  TEST (FuseContentSelection, rejects_implicit_storage)
  {
    mcs::testing::require_exception
      ( []
        {
          selects<mcs::fuse::content::implementation::Vector>
            ( {"Heap (Limit 1048576)"}
            );
        }
      , mcs::testing::assert_type<std::invalid_argument>()
      );
  }

  TEST (FuseContentSelection, rejects_incomplete_storage_arguments)
  {
    mcs::testing::require_exception
      ( []
        {
          selects<mcs::fuse::content::implementation::Vector>
            ( {"Storage", "Heap (Unlimited)"}
            );
        }
      , mcs::testing::assert_type<std::invalid_argument>()
      );
  }

  TEST (FuseContentSelection, honors_the_callers_storage_list)
  {
    using HeapOnly = mcs::util::type::List
      < mcs::core::storage::implementation::Heap
      >;

    auto arguments
      { std::vector<char const*>
        { "Storage"
        , "Files (Prefix \"/tmp\", Limit 1048576)"
        , ""
        , ""
        , ""
        , ""
        , ""
        }
      };

    mcs::testing::require_exception
      ( [&]
        {
          mcs::fuse::content::selection::select<HeapOnly>
            ( mcs::util::arguments::View {arguments.data(), arguments.size()}
            , []<mcs::fuse::is_content Content>
                ( typename Content::State
                , mcs::util::arguments::View
                )
              {}
            );
        }
      , mcs::testing::assert_type<mcs::util::read::ParseError>()
      );
  }
}

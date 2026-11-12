// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fmt/format.h>
#include <iterator>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/core/storage/implementation/Virtual.hpp>
#include <mcs/fuse/content/select.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/main.hpp>
#include <mcs/fuse/option.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/option/Parameter.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/type/List.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    using mcs::fuse::option::content;
    using mcs::fuse::option::fuse_embedding;
    using mcs::fuse::option::mountpoint;
    using mcs::fuse::option::storage;
    using mcs::fuse::option::storage_chunk_description;
    using mcs::fuse::option::storage_segment_create;
    using mcs::fuse::option::storage_segment_remove;
    using mcs::fuse::option::storage_size_max;
    using mcs::fuse::option::storage_size_used;
    constexpr auto name {Parameter {"name"}};
  }

  using SupportedStorageImplementations = mcs::util::type::List
    < mcs::core::storage::implementation::Files
    , mcs::core::storage::implementation::Heap
    , mcs::core::storage::implementation::SHMEM
    , mcs::core::storage::implementation::Virtual
    >;

  auto print_usage()
  {
    mcs::util::FMT::print_noexcept
      ( stdout
      , R"(Usage:
  mcs_fuse_bin_fs --{0} CONTENT --{1} NAME --{2} MOUNTPOINT
    [STORAGE_OPTIONS]

Optional native arguments:
  '[[{9}' FUSE_OPTIONS '{9}]]' (repeatable; closing tag required)

Expose file content through FUSE.

Values:
  CONTENT  'Vector', 'Realloc', or 'Storage'

'Storage' options (all required for --{0} Storage):
  --{3} EXPRESSION
  --{4} EXPRESSION
  --{5} EXPRESSION
  --{6} EXPRESSION
  --{7} EXPRESSION
  --{8} EXPRESSION

Examples:
  mcs_fuse_bin_fs --{0} Vector --{1} mcs-vector \
    --{2} /mnt/mcs '[[{9}' -f '{9}]]'

  mcs_fuse_bin_fs --{0} Storage --{1} mcs-heap \
    --{3} 'Heap (Limit 1073741824)' \
    --{4} 'Heap::Size::Max()' \
    --{5} 'Heap::Size::Used()' \
    --{6} 'Heap::Segment::Create (Nothing)' \
    --{7} 'Heap::Segment::Remove()' \
    --{8} 'Heap::Chunk::Description()' \
    --{2} /mnt/mcs '[[{9}' -f '{9}]]'

  )"
      , option::content
      , option::name
      , option::mountpoint
      , option::storage
      , option::storage_size_max
      , option::storage_size_used
      , option::storage_segment_create
      , option::storage_segment_remove
      , option::storage_chunk_description
      , option::fuse_embedding
      );

    ::fuse_cmdline_help();
    ::fuse_lowlevel_help();
  }

  auto fs_main (mcs::util::arguments::View arguments) -> int
  {
    auto const schema
      { mcs::fuse::content::selection::options::schema()
      . add (option::name)
      . add (option::mountpoint)
      . embedding (option::fuse_embedding)
      };

    auto const parsed {arguments.from (1UL).parse (schema)};
    auto fuse_arguments
      { std::vector<char const*> {parsed.at (option::name).data()}
      };
    std::ranges::transform
      ( parsed.embedded (option::fuse_embedding)
      , std::back_inserter (fuse_arguments)
      , [] (std::string_view argument) noexcept
        {
          return argument.data();
        }
      );
    fuse_arguments.emplace_back
      ( parsed.at (option::mountpoint).data()
      );

    return mcs::fuse::content::selection::select
      < SupportedStorageImplementations
      >
      ( mcs::fuse::content::selection::Arguments {parsed}
      , [&]<mcs::fuse::is_content Content>
          (typename Content::State state)
        {
          return mcs::fuse::main<Content>
            ( print_usage
            , mcs::util::arguments::View
              { fuse_arguments.data(), fuse_arguments.size()
              }
            , std::move (state)
            );
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main
    ( argc
    , argv
    , [] (mcs::util::arguments::View)
      {
        print_usage();
      }
    , fs_main
    );
}

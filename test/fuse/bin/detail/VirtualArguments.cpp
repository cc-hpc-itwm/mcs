// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdlib>
#include <fmt/format.h>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/Virtual.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/main.hpp>

namespace
{
  auto help (mcs::util::arguments::View args)
  {
    fmt::print
      ( "Usage: {}\n"
        "Print the Virtual<Heap> storage parameters for FUSE tests.\n"
        "No arguments are required.\n"
      , args.at (0)
      );
  }

  auto virtual_arguments_main (mcs::util::arguments::View) -> int
  {
    using Heap = mcs::testing::core::storage::implementation::Heap;
    using Virtual = mcs::testing::core::storage::implementation::Virtual<Heap>;

    auto const storage
      { Virtual
        { mcs::core::storage::MaxSize::Limit
          { mcs::core::memory::make_size (16777216UL)
          }
        }
      };

    fmt::print
      ( "{}\n{}\n{}\n{}\n{}\n{}\n"
      , storage.parameter_create()
      , storage.parameter_size_max()
      , storage.parameter_size_used()
      , storage.parameter_segment_create()
      , storage.parameter_segment_remove()
      , storage.parameter_chunk_description()
      );
    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, help, virtual_arguments_main);
}

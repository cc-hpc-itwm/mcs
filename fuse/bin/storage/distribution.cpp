// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/base.h>
#include <fmt/format.h>
#include <mcs/fuse/content/implementation/storage/for_each_part.hpp>
#include <mcs/util/main.hpp>
#include <stdexcept>

namespace
{
  auto mcs_distribution_main (mcs::util::Args args) -> int
  {
    if (args.size() != 2)
    {
      throw std::invalid_argument
        { fmt::format ("usage: {} filename", args[0])
        };
    }

    mcs::fuse::content::storage::for_each_part
      ( args[1]
      , [] (auto const& part)
        {
          fmt::print ("{}\n", part);
        }
      );

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, mcs_distribution_main);
}

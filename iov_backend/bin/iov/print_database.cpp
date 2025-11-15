// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/IOV_Database.hpp>
#include <mcs/IOV_Meta.hpp>
#include <mcs/util/ExistingPath.hpp>
#include <mcs/util/main.hpp>
#include <stdexcept>

namespace
{
  auto print_database_main (mcs::util::Args args) -> int
  {
    if (args.size() != 2)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {}"
                        " database_file"                    // 1
                      , args[0]
                      )
        };
    }

    auto const database_file {mcs::util::ExistingPath {args[1]}};
    auto const database {mcs::IOV_Database {*database_file}};

    for ( auto const& [key, value]
        : database.iov_key_value_map (database.read_access())
        )
    {
      fmt::print ("{} -> {}\n", key, value);
    }

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, print_database_main);
}

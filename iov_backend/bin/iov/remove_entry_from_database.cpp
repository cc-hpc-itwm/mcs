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
  auto remove_entry_from_database_main (mcs::util::Args args) -> int
  {
    if (args.size() != 3)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {}"
                        " database_file"                    // 1
                        " key"                              // 2
                      , args[0]
                      )
        };
    }

    auto const database_file {mcs::util::ExistingPath {args[1]}};
    auto const key {iov::meta::Key {args[2]}};

    auto database {mcs::make_scoped_iov_database (*database_file)};

    auto const value {database->remove (database->write_access(), key)};

    fmt::print ("{}\n", value);

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, remove_entry_from_database_main);
}

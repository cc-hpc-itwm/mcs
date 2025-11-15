// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "workspace_name.hpp"
#include <fmt/format.h>
#include <mcs/IOV_Database.hpp>
#include <mcs/iov_backend/invoke_and_throw_on_unexpected.hpp>
#include <mcs/util/NonExistingPath.hpp>
#include <mcs/util/main.hpp>
#include <memory>
#include <stdexcept>

namespace
{
  auto make_empty_database_main (mcs::util::Args args) -> int
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

    auto const database_file {mcs::util::NonExistingPath {args[1]}};

    auto database {mcs::IOV_Database{}};
    auto database_ref {mcs::IOV_DBRef {std::addressof (database)}};

    std::ignore = mcs::iov_backend::invoke_and_throw_on_unexpected
      ( iov::meta::MetaData::create_workspace
      , fmt::format ("Could not create workspace")
      , mcs::iov_backend::workspace_name()
      , std::addressof (database_ref)
      );

    database.save (*database_file);

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, make_empty_database_main);
}

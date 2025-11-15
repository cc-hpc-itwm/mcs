// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "workspace_name.hpp"
#include <fmt/format.h>
#include <iov/iov.hpp>
#include <mcs/IOV_Backend.hpp>
#include <mcs/IOV_Database.hpp>
#include <mcs/IOV_Meta.hpp>
#include <mcs/IOV_UUID.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/iov_backend/invoke_and_throw_on_unexpected.hpp>
#include <mcs/util/ExistingPath.hpp>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/read/uint.hpp>
#include <stdexcept>

namespace
{
  auto collection_delete_main (mcs::util::Args args) -> int
  {
    if (args.size() != 4)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {}"
                        " configuration_file"               // 1
                        " database_file"                    // 2
                        " collection_id"                    // 3
                      , args[0]
                      )
        };
    }

    auto const configuration_file {mcs::util::ExistingPath {args[1]}};
    auto const database_file {mcs::util::ExistingPath {args[2]}};
    auto const collection_id {mcs::iov_backend::collection::ID {args[3]}};

    auto database {mcs::make_scoped_iov_database (*database_file)};
    auto database_ref {mcs::IOV_DBRef {std::addressof (*database)}};

    {
      auto meta_data
        { mcs::iov_backend::invoke_and_throw_on_unexpected
            ( iov::meta::MetaData::load_collection
            , fmt::format
                ( "IOV: Could not load collection '{}'"
                  " in workspace '{}' in database '{}'"
                , collection_id._uuid
                , mcs::iov_backend::workspace_name()
                , *database_file
                )
            , collection_id._uuid
            , mcs::iov_backend::workspace_name()
            , std::addressof (database_ref)
            )
        };

      auto iov_backend {mcs::IOV_Backend {configuration_file}};

      mcs::iov_backend::invoke_and_throw_on_unexpected
        ( [&]
          {
            return iov_backend.collection_delete
              ( iov::DeleteParam
                { .name = iov::meta::Name {"ignored_by_backend_ops"}
                , .meta_data = std::addressof (meta_data)
                }
              ).get().result;
          }
        , fmt::format
            ( "Backend: Could not delete collection {}"
            , meta_data.uuid()
            )
        );
    }

    mcs::iov_backend::invoke_and_throw_on_unexpected
      ( iov::meta::MetaData::delete_collection
      , fmt::format
          ( "IOV: Could not load collection '{}'"
            " in workspace '{}' in database '{}'"
          , mcs::iov_backend::workspace_name()
          , collection_id._uuid
          , *database_file
          )
      , collection_id._uuid
      , mcs::iov_backend::workspace_name()
      , std::addressof (database_ref)
      );

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, collection_delete_main);
}

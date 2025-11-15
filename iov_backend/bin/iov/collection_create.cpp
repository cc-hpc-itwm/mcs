// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "workspace_name.hpp"
#include <cstdio>
#include <exception>
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
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/read/uint.hpp>
#include <stdexcept>

namespace
{
  auto collection_create_main (mcs::util::Args args) -> int
  {
    if (args.size() != 6)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {}"
                        " configuration_file"             // 1
                        " database_file"                  // 2
                        " collection_id"                  // 3
                        " size_min(0=unknown)"            // 4
                        " size_max(0=unknown)"            // 5
                      , args[0]
                      )
        };
    }

    auto const configuration_file {mcs::util::ExistingPath {args[1]}};
    auto const database_file {mcs::util::ExistingPath {args[2]}};
    auto const collection_id {mcs::iov_backend::collection::ID {args[3]}};
    auto const size_min {mcs::util::read::read<std::size_t> (args[4])};
    auto const size_max {mcs::util::read::read<std::size_t> (args[5])};

    auto const make_limit
      { [] (auto size) -> iov::size::limit::Size
        {
          if (std::cmp_equal (size, 0))
          {
            return iov::size::limit::Unknown{};
          }

          return iov::size::limit::Value {size};
        }
      };

    auto database {mcs::make_scoped_iov_database (*database_file)};
    auto database_ref {mcs::IOV_DBRef {std::addressof (*database)}};

    auto collection_meta_data
      { mcs::iov_backend::invoke_and_throw_on_unexpected
        ( iov::meta::MetaData::create_collection
        , fmt::format
            ( "IOV: Could not create collection '{}'"
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
    auto iov_backend_context {iov_backend.context (std::addressof (*database))};

    auto const workspace_meta_data
      { mcs::iov_backend::invoke_and_throw_on_unexpected
          ( iov::meta::MetaData::load_workspace
          , fmt::format
              ( "IOV: Could not create metadata for workspace '{}'"
                " in database '{}'"
              , mcs::iov_backend::workspace_name()
              , *database_file
              )
          , mcs::iov_backend::workspace_name()
          , std::addressof (database_ref)
          )
      };

    auto const collection_open_result
      { iov_backend.collection_open
        ( iov::CollectionOpenParam
          { .name = iov::meta::Name {"ignored_by_backend_ops"}
          , .meta_data = std::addressof (collection_meta_data)
          , .controls = iov::CollectionControls
            { iov::concurrency::Exclusive{}
            , iov::consistency::Sequential{}
            , iov::mutability::Immutable{}
            , iov::temporality::Once{}
            , iov::Size_Ctl
              { .request_size = iov::size::Size
                  { .min = iov::size::limit::Unknown{}
                  , .max = iov::size::limit::Unknown{}
                  }
              , .collection_size = iov::size::Size
                  { .min = make_limit (size_min)
                  , .max = make_limit (size_max)
                  }
              }
            }
          , .workspace = iov::Workspace
            { workspace_meta_data
            , iov::WorkspaceControls
                { iov::concurrency::Exclusive{}
                , iov::consistency::Commit{}
                , iov::mutability::Immutable{}
                }
            , std::addressof (iov_backend_context)
            }
          }
        )
      };

    if (!collection_open_result)
    {
      try
      {
        mcs::iov_backend::invoke_and_throw_on_unexpected
          ( iov::meta::MetaData::delete_collection
          , fmt::format
              ( "IOV: Could not delete collection '{}'"
                " from workspace '{}' in database '{}'"
              , collection_id._uuid
              , mcs::iov_backend::workspace_name()
              , *database_file
              )
          , collection_id._uuid
          , mcs::iov_backend::workspace_name()
          , std::addressof (database_ref)
          );
      }
      catch (...)
      {
        mcs::util::FMT::print_noexcept
          ( stderr
          , "Error during Rollback: {}\n"
          , std::current_exception()
          );
      }

      throw std::runtime_error
        { fmt::format ("Error: {}", collection_open_result.error().reason())
        };
    }

    fmt::print ("{}\n", collection_meta_data.name());
    fmt::print ("{}\n", collection_meta_data.uuid());

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, collection_create_main);
}

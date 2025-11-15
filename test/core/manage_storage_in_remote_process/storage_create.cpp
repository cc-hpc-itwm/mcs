// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "SupportedStorageImplementations.hpp"
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/core/operator==/storage/MaxSize.hpp>
#include <mcs/testing/core/printer/memory/Size.hpp>
#include <mcs/testing/core/printer/storage/MaxSize.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/FMT/write_file.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/STD/filesystem/path.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  auto storage_create_main (mcs::util::Args args) -> int
  {
    if (args.size() != 4)
    {
      throw std::invalid_argument
        { fmt::format
            ( "usage: {} provider_connectable max_size output_file"
            , args[0]
            )
        };
    }

    auto const provider_connectable
      {mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args[1])};
    auto const max_size
      {mcs::util::read::read<mcs::core::storage::MaxSize> (args[2])};
    auto const output_file
      {mcs::util::read::read<std::filesystem::path> (args[3])};

    auto io_context
      { mcs::rpc::ScopedRunningIOContext
          {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM}
      };

    return mcs::util::ASIO::run
      ( provider_connectable
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (mcs::util::ASIO::Connectable<Protocol> connectable)
        {
          auto const client
            { mcs::core::control::Client
                  < Protocol
                  , mcs::rpc::access_policy::Exclusive
                  , SupportedStorageImplementations
                  >
                { io_context
                , std::move (connectable)
                }
            };

          using Storage = mcs::core::storage::implementation::SHMEM;

          // Create storage at the remote storages
          //
          auto const storage_id
            { client
            . template storage_create<Storage>
              ( typename Storage::Parameter::Create
                { typename Storage::Prefix
                  { mcs::util::string
                    {"MCS_CORE_TEST_MANAGE_STORAGE_IN_REMOTE_PROCESS_STORAGE_CREATE"}
                  }
                , max_size
                }
              )
            };

          // Query the storage at the remote controller
          //
          if (! (  max_size
                == client.storage_size_max
                     ( storage_id
                     , mcs::core::storage::make_parameter
                         (typename Storage::Parameter::Size::Max{})
                     )
                )
             )
          {
            return EXIT_FAILURE;
          }

          if (! (  mcs::core::memory::make_size (0)
                == client.storage_size_used
                     ( storage_id
                     , mcs::core::storage::make_parameter
                         (typename Storage::Parameter::Size::Used{})
                     )
                )
             )
          {
            return EXIT_FAILURE;
          }

          // Publish the storage_id to be used by remote processes
          //
          mcs::util::FMT::write_file (output_file, "{}", storage_id);

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, storage_create_main);
}

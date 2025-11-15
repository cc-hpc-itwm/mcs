// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "SupportedStorageImplementations.hpp"
#include <csignal>
#include <cstdlib>
#include <fmt/format.h>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  auto storage_remove_main (mcs::util::Args args) -> int
  {
    if (args.size() != 3)
    {
      throw std::invalid_argument
        { fmt::format
            ( "usage: {} provider_connectable storage_id"
            , args[0]
            )
        };
    }

    auto const provider_connectable
      {mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args[1])};
    auto const storage_id
      {mcs::util::read::read<mcs::core::storage::ID> (args[2])};

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

          client.storage_remove (storage_id);

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, storage_remove_main);
}

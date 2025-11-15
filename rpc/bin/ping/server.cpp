// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "Dispatcher.hpp"
#include <csignal>
#include <cstdlib>
#include <fmt/format.h>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/Endpoint.hpp>
#include <mcs/util/TemporaryDirectory.hpp>
#include <mcs/util/TemporaryFile.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <stdexcept>
#include <utility>

namespace
{
  auto server_main (mcs::util::Args args) -> int
  {
    if (args.size() != 4)
    {
      throw std::invalid_argument
        { fmt::format
          ( "usage: {} endpoint ping_service_path number_of_threads"
          , args[0]
          )
        };
    }

    auto const number_of_threads
      { mcs::util::read::read<unsigned int> (args[3])
      };
    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {number_of_threads}
        , SIGINT, SIGTERM
        }
      };

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::Endpoint> (args[1])
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (typename Protocol::endpoint endpoint)
        {
          auto const temporary_directory
            { mcs::util::TemporaryDirectory {args[2]}
            };

          auto const provider
            { mcs::rpc::make_provider<Protocol, mcs::rpc::ping::Dispatcher>
              ( endpoint
              , io_context
              )
            };

          auto const connectable_information
            { mcs::util::TemporaryFile
              { temporary_directory.path() / "SERVER"
              , "{}"
              , mcs::util::ASIO::make_connectable (provider.local_endpoint())
              }
            };

          auto const pid_information
            { mcs::util::TemporaryFile
              { temporary_directory.path() / "PID"
              , "{}"
              , mcs::util::syscall::getpid()
              }
            };

          io_context.join();

          // \todo daemonize and proper return once the connection
          // information has been written

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, server_main);
}

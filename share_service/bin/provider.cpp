// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <fmt/format.h>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/share_service/Provider.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/Endpoint.hpp>
#include <mcs/util/TemporaryDirectory.hpp>
#include <mcs/util/TemporaryFile.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <stdexcept>
#include <utility>

namespace
{
  auto provider_main (mcs::util::Args args) -> int
  {
    if (args.size() != 4)
    {
      throw std::invalid_argument
        { fmt::format
          ( "usage: {} endpoint share_service_path number_of_threads"
          , args[0]
          )
        };
    }

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::Endpoint> (args[1])
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (typename Protocol::endpoint endpoint)
        {
          auto const temporary_directory
            { mcs::util::TemporaryDirectory {args[2]}
            };

          auto io_context
            { mcs::rpc::ScopedRunningIOContext
                { mcs::util::read::read<mcs::rpc::ScopedRunningIOContext::NumberOfThreads>
                    (args[3])
                , SIGINT, SIGTERM
                }
            };

          auto provider
            { mcs::share_service::Provider<Protocol>
                { io_context
                , endpoint
                }
            };

          auto const provider_information
            { mcs::util::TemporaryFile
              { temporary_directory.path() / "PROVIDER"
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
  return mcs::util::main (argc, argv, provider_main);
}

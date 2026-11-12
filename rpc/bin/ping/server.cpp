// Copyright (C) 2022-2026 Fraunhofer ITWM
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
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/daemon.hpp>
#include <mcs/util/daemon/option.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <stdexcept>
#include <utility>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    constexpr auto endpoint {Parameter {"endpoint"}};
    constexpr auto number_of_threads {Parameter {"number-of-threads"}};
    constexpr auto ping_service_path {Parameter {"ping-service-path"}};
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <ENDPOINT>\n"
        "    Ping-service listening endpoint.\n"
        "  --{2} <PATH>\n"
        "    Directory for publishing ping-server information.\n"
        "  --{3} <COUNT>\n"
        "    Number of RPC I/O threads.\n"
      , args.at (0)
      , option::endpoint
      , option::ping_service_path
      , option::number_of_threads
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print
      ( "{0}\nOptional settings:\n"
        "  {1}  Run in foreground (default: background)\n"
      , usage (args)
      , mcs::util::daemon::option::foreground
      );
  }

  auto server_main
    ( mcs::util::arguments::View args
    , mcs::util::Daemon& daemon
    ) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::endpoint)
        . add (option::ping_service_path)
        . add (option::number_of_threads)
        )
      };
    auto const number_of_threads
      { mcs::util::read::read<unsigned int>
          ( options.at (option::number_of_threads)
          )
      };
    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {number_of_threads}
        , SIGINT, SIGTERM
        }
      };

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::Endpoint>
          ( options.at (option::endpoint)
          )
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (typename Protocol::endpoint endpoint)
        {
          auto const temporary_directory
            { mcs::util::TemporaryDirectory
              { options.at (option::ping_service_path)
              }
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

          daemon.ready();

          io_context.join();

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::daemonize (argc, argv, help, server_main);
}

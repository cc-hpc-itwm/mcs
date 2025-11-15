// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <fmt/format.h>
#include <mcs/block_device/block/Size.hpp>
#include <mcs/block_device/meta_data/Blocks.hpp>
#include <mcs/block_device/meta_data/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/Endpoint.hpp>
#include <mcs/util/TemporaryDirectory.hpp>
#include <mcs/util/TemporaryFile.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <memory>
#include <stdexcept>
#include <utility>

namespace
{
  auto meta_data_provider_main (mcs::util::Args args) -> int
  {
    if (args.size() != 5)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {} meta_data_endpoint meta_data_provider_path block_size number_of_threads"
                      , args[0]
                      )
        };
    }

    auto blocks
      { mcs::block_device::meta_data::Blocks
         {mcs::util::read::read<mcs::block_device::block::Size> (args[3])}
      };
    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
            {mcs::util::read::read<unsigned int> (args[4])}
        , SIGINT, SIGTERM
        }
      };

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::Endpoint> (args[1])
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (typename Protocol::endpoint meta_data_endpoint)
        {
          auto const provider
            { mcs::block_device::meta_data::Provider<Protocol>
              { meta_data_endpoint
              , io_context
              , std::addressof (blocks)
              }
            };

          auto const temporary_directory
            {mcs::util::TemporaryDirectory {args[2]}};

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

          // run "for ever"
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
  return mcs::util::main (argc, argv, meta_data_provider_main);
}

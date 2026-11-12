// Copyright (C) 2023-2026 Fraunhofer ITWM
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
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/daemon.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <memory>
#include <stdexcept>
#include <utility>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    constexpr auto block_size {Parameter {"block-size"}};
    constexpr auto metadata_endpoint {Parameter {"metadata-endpoint"}};
    constexpr auto metadata_provider_path
      { Parameter {"metadata-provider-path"}
      };
    constexpr auto number_of_threads {Parameter {"number-of-threads"}};
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <ENDPOINT>\n"
        "    Metadata-service listening endpoint.\n"
        "  --{2} <PATH>\n"
        "    Directory for publishing metadata-provider information.\n"
        "  --{3} <SIZE_IN_BYTES>\n"
        "    Size of each block.\n"
        "  --{4} <COUNT>\n"
        "    Number of RPC I/O threads.\n"
      , args.at (0)
      , option::metadata_endpoint
      , option::metadata_provider_path
      , option::block_size
      , option::number_of_threads
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}\n", usage (args));
  }

  auto meta_data_provider_main
    ( mcs::util::arguments::View args
    , mcs::util::Daemon& daemon
    ) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::metadata_endpoint)
        . add (option::metadata_provider_path)
        . add (option::block_size)
        . add (option::number_of_threads)
        )
      };
    auto blocks
      { mcs::block_device::meta_data::Blocks
        { mcs::util::read::read<mcs::block_device::block::Size>
            ( options.at (option::block_size)
            )
        }
      };
    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
            { mcs::util::read::read<unsigned int>
                ( options.at (option::number_of_threads)
                )
            }
        , SIGINT, SIGTERM
        }
      };

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::Endpoint>
          ( options.at (option::metadata_endpoint)
          )
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
            { mcs::util::TemporaryDirectory
              { options.at (option::metadata_provider_path)
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

          daemon.ready();

          io_context.join();

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::daemonize (argc, argv, help, meta_data_provider_main);
}

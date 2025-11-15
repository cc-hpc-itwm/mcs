// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <mcs/block_device/meta_data/Client.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>
#include <utility>

namespace
{
  auto number_of_blocks_main (mcs::util::Args args) -> int
  {
    if (args.size() != 2)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {} meta_data_provider_path"
                      , args[0]
                      )
        };
    }

    auto const meta_data_provider_path {std::filesystem::path (args[1])};
    auto const meta_data_provider_connectable
      { mcs::util::read::from_file<mcs::util::ASIO::AnyConnectable>
          (meta_data_provider_path / "PROVIDER")
      };

    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        , SIGINT, SIGTERM
        }
      };

    return mcs::util::ASIO::run
      ( meta_data_provider_connectable
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (mcs::util::ASIO::Connectable<Protocol> meta_data_provider)
        {
          auto meta_data_client
            { mcs::block_device::meta_data::Client
                  < Protocol
                  , mcs::rpc::access_policy::Exclusive
                  >
                { io_context
                , meta_data_provider
                }
            };

          fmt::print ("{}\n", meta_data_client.number_of_blocks());

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, number_of_blocks_main);
}

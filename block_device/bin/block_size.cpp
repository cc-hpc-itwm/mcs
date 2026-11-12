// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <mcs/block_device/meta_data/Client.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>
#include <utility>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    constexpr auto metadata_provider_path
      { Parameter {"metadata-provider-path"}
      };
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <PATH>\n"
        "    Directory containing metadata-provider information.\n"
      , args.at (0)
      , option::metadata_provider_path
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}\n", usage (args));
  }

  auto block_size_main (mcs::util::arguments::View args) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::metadata_provider_path)
        )
      };
    auto const meta_data_provider_path
      { std::filesystem::path {options.at (option::metadata_provider_path)}
      };
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

          fmt::print ("{}\n", meta_data_client.block_size());

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, help, block_size_main);
}

// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <mcs/block_device/block/Range.hpp>
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

    constexpr auto block_range {Parameter {"block-range"}};
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
        "  --{2} <block::Range>\n"
        "    Range of blocks to remove.\n"
      , args.at (0)
      , option::metadata_provider_path
      , option::block_range
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}\n", usage (args));
  }

  auto remove_main (mcs::util::arguments::View args) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::metadata_provider_path)
        . add (option::block_range)
        )
      };
    auto const meta_data_provider_path
      { std::filesystem::path {options.at (option::metadata_provider_path)}
      };
    auto const meta_data_provider_connectable
      { mcs::util::read::from_file<mcs::util::ASIO::AnyConnectable>
          (meta_data_provider_path / "PROVIDER")
      };

    auto const block_range
      { mcs::util::read::read<mcs::block_device::block::Range>
          ( options.at (option::block_range)
          )
      };

    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        , SIGINT, SIGTERM
        }
      };

    return mcs::util::ASIO::run
      ( meta_data_provider_connectable
      , [&]<mcs::util::ASIO::is_protocol MetaDataProtocol>
          (mcs::util::ASIO::Connectable<MetaDataProtocol> meta_data_provider)
        {
          auto meta_data_client
            { mcs::block_device::meta_data::Client
                  < MetaDataProtocol
                  , mcs::rpc::access_policy::Exclusive
                  >
                { io_context
                , meta_data_provider
                }
            };

          auto const remove_result {meta_data_client.remove (block_range)};

          fmt::print ("{}\n", remove_result.unused);

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, help, remove_main);
}

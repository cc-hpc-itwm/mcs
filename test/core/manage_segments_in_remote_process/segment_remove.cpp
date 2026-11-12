// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "SupportedStorageImplementations.hpp"
#include <csignal>
#include <cstdlib>
#include <fmt/format.h>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>
#include <tuple>

namespace
{
  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {} provider_connectable storage_id storage_parameter"
        " segment_id"
      , args.at (0)
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}\n", usage (args));
  }

  auto segment_remove_main (mcs::util::arguments::View args) -> int
  {
    if (args.size() != 5)
    {
      throw std::invalid_argument {usage (args)};
    }

    auto const provider_connectable
      {mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args.at (1))};
    auto const storage_id
      {mcs::util::read::read<mcs::core::storage::ID> (args.at (2))};
    auto const storage_parameter
      {mcs::util::read::read<mcs::core::storage::Parameter> (args.at (3))};
    auto const segment_id
      {mcs::util::read::read<mcs::core::storage::segment::ID> (args.at (4))};

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

          // Remove segment at the remote storages
          //
          std::ignore = client.segment_remove
            ( storage_id
            , storage_parameter
            , segment_id
            );

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, help, segment_remove_main);
}

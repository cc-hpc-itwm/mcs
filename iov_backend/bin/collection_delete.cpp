// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <fmt/format.h>
#include <mcs/core/memory/Size.hpp>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  auto collection_delete_main (mcs::util::Args args) -> int
  {
    if (args.size() != 3)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {} provider collection_id"
                      , args[0]
                      )
        };
    }

    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        , SIGINT, SIGTERM
        }
      };

    std::visit
      ( [&] (auto&& client)
        {
          return client
            . collection_delete
                ( mcs::iov_backend::collection::ID {args[2]}
                )
            ;
        }
      , mcs::iov_backend::make_client
            < mcs::rpc::access_policy::Exclusive
            >
          ( io_context
          , mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args[1])
          )
      );

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, collection_delete_main);
}

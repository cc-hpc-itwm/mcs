// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <fmt/format.h>
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
  auto range_main (mcs::util::Args args) -> int
  {
    if (args.size() != 3)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {} provider collection_id"
                      , args[0]
                      )
        };
    }

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args[1])
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          ( mcs::util::ASIO::Connectable<Protocol> provider_connectable
          )
        {
          auto io_context
            { mcs::rpc::ScopedRunningIOContext
              { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
              , SIGINT, SIGTERM
              }
            };

          auto const client
            { mcs::iov_backend::Client
                < Protocol
                , mcs::rpc::access_policy::Exclusive
                >
              { io_context
              , provider_connectable
              }
            };

          fmt::print
            ( "{}\n"
            , client.range (mcs::iov_backend::collection::ID {args[2]})
            );

          return EXIT_SUCCESS;
        }
      );

  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, range_main);
}

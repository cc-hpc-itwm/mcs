// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  auto state_main (mcs::util::Args args) -> int
  {
    if (args.size() != 2)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {} provider"
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

    auto const state
      { std::visit
        ( [&] (auto&& client)
          {
            return client.provider_state();
          }
        , mcs::iov_backend::make_client
              < mcs::rpc::access_policy::Exclusive
              >
          ( io_context
          , mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args[1])
          )
        )
      };

    std::cout << mcs::serialization::OArchive {state};

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, state_main);
}

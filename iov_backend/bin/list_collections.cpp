// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <fmt/format.h>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  auto list_collections_main (mcs::util::Args args) -> int
  {
    if (args.size() != 2)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {} provider"
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

          for (auto const& [id, used_storages] : client.provider_state().collections())
          {
            fmt::print ("collection: {}\n", id);

            for (auto const& [segment_id, range, storage_id] : used_storages)
            {
              fmt::print ( "  - range: {}, storage: {}, id: {}\n"
                         , range
                         , storage_id
                         , segment_id
                         );
            }
          }

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, list_collections_main);
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <mcs/iov_backend/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/util/ASIO/Endpoint.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <memory>
#include <stdexcept>

namespace
{
  auto provider_main (mcs::util::Args args) -> int
  {
    if (args.size() != 3 && args.size() != 4)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {} endpoint number_of_threads [State]"
                      , args[0]
                      )
        };
    }

    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
            {mcs::util::read::read<unsigned int> (args[2])}
        , SIGINT, SIGTERM
        }
      };

    auto io_context_storages_clients
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u} // \todo parameter
        , SIGINT, SIGTERM
        }
      };

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::Endpoint> (args[1])
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (typename Protocol::endpoint endpoint)
        {
          auto storages_clients
            { mcs::iov_backend::provider::StoragesClients{}
            };
          auto const provider
            { mcs::iov_backend::Provider
                  < Protocol
                  , decltype (io_context_storages_clients)
                  >
                { endpoint
                , io_context
                , io_context_storages_clients
                , std::addressof (storages_clients)
                , std::invoke
                  ( [&]
                    {
                      using State = mcs::iov_backend::provider::State;
                      if (args.size() == 4)
                      {
                        return mcs::serialization::load_from<State>
                          ( std::filesystem::path {args[3]}
                          )
                          . restart ( io_context_storages_clients
                                    , std::addressof (storages_clients)
                                    )
                          ;
                      }

                      return State{};
                    }
                  )
                }
            };

          fmt::print
            ( "{}\n"
            , mcs::util::ASIO::make_connectable (provider.local_endpoint())
            );

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
  return mcs::util::main (argc, argv, provider_main);
}

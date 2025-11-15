// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "Dispatcher.hpp"
#include <chrono>
#include <filesystem>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <functional>
#include <list>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/multi_client/ParallelCallsLimit.hpp>
#include <mcs/rpc/multi_client/call.hpp>
#include <mcs/rpc/multi_client/command_generator/Reference.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/read/uint.hpp>
#include <stdexcept>

namespace
{
  template<mcs::rpc::is_command Command>
    struct DroppingCollector
  {
    template<typename ClientID>
      constexpr auto result
        ( ClientID const&
        , typename Command::Response
        ) const noexcept
    {}
  };
}

namespace
{
  auto client_main (mcs::util::Args args) -> int
  {
    if (args.size() != 6)
    {
      throw std::invalid_argument
        {fmt::format ( "usage: {} ping_service_path message_size repetitions number_of_clients number_of_threads"
                     , args[0]
                     )
        };
    }

    auto const ping_service_path {std::filesystem::path (args[1])};
    auto const message_size {mcs::util::read::read<unsigned> (args[2])};
    auto const repetitions {mcs::util::read::read<unsigned> (args[3])};
    auto const number_of_clients {mcs::util::read::read<unsigned> (args[4])};

    return mcs::util::ASIO::run
      ( mcs::util::read::from_file<mcs::util::ASIO::AnyConnectable>
          (ping_service_path / "SERVER")
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (mcs::util::ASIO::Connectable<Protocol> connectable)
        {
          auto io_context
            { mcs::rpc::ScopedRunningIOContext
              { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
                  {mcs::util::read::read<unsigned> (args[5])}
              , SIGINT, SIGTERM
              }
            };

          using Clock = std::chrono::steady_clock;
          using Ticks = std::chrono::microseconds;

          auto start = Clock::now();

          using Client = typename mcs::rpc::ping::Dispatcher::ClientType
            < Protocol
            , mcs::rpc::access_policy::Exclusive
            >;
          auto clients {std::list<std::variant<Client>>{}};

          for (auto i {0u}; i < number_of_clients; ++i)
          {
            clients.emplace_back
              ( mcs::rpc::make_client< Protocol
                                     , mcs::rpc::ping::Dispatcher
                                     , mcs::rpc::access_policy::Exclusive
                                     >
                  ( io_context
                  , connectable
                  )
              );
          }
          auto end = Clock::now();
          {
            auto const duration
              {std::chrono::duration_cast<Ticks> (end - start)};

            fmt::print ( "create {} clients: {} -> {} per client\n"
                       , number_of_clients
                       , duration
                       , duration / number_of_clients
                       );
          }

          auto const command {mcs::rpc::ping::Ping {message_size}};

          start = Clock::now();
          for (auto i {0u}; i < repetitions; ++i)
          {
            mcs::rpc::multi_client::call< DroppingCollector
                                        , mcs::rpc::ping::Ping
                                        >
              ( mcs::rpc::multi_client::command_generator::Reference
                  < mcs::rpc::ping::Ping
                  >
                    {command}
              , clients
              , mcs::rpc::multi_client::ParallelCalls::Unlimited{}
              );
          }
          end = Clock::now();
          {
            auto const duration
              {std::chrono::duration_cast<Ticks> (end - start)};

            fmt::print
              ( "ping ({}x{} times {} bytes): {} -> {} per roundtrip = {} MB/sec\n"
              , number_of_clients
              , repetitions
              , message_size
              , duration
              , duration / repetitions / number_of_clients
              , message_size / (duration / repetitions / number_of_clients).count()
              );
          }

          return EXIT_SUCCESS;
        }
     );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, client_main);
}

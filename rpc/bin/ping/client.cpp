// Copyright (C) 2022-2026 Fraunhofer ITWM
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
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    constexpr auto message_size {Parameter {"message-size"}};
    constexpr auto number_of_clients {Parameter {"number-of-clients"}};
    constexpr auto number_of_threads {Parameter {"number-of-threads"}};
    constexpr auto ping_service_path {Parameter {"ping-service-path"}};
    constexpr auto repetitions {Parameter {"repetitions"}};
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <PATH>\n"
        "    Directory containing ping-server information.\n"
        "  --{2} <SIZE_IN_BYTES>\n"
        "    Size of each ping payload.\n"
        "  --{3} <COUNT>\n"
        "    Number of ping repetitions.\n"
        "  --{4} <COUNT>\n"
        "    Number of clients.\n"
        "  --{5} <COUNT>\n"
        "    Number of RPC I/O threads.\n"
      , args.at (0)
      , option::ping_service_path
      , option::message_size
      , option::repetitions
      , option::number_of_clients
      , option::number_of_threads
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}\n", usage (args));
  }

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
  auto client_main (mcs::util::arguments::View args) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::ping_service_path)
        . add (option::message_size)
        . add (option::repetitions)
        . add (option::number_of_clients)
        . add (option::number_of_threads)
        )
      };
    auto const ping_service_path
      { std::filesystem::path {options.at (option::ping_service_path)}
      };
    auto const message_size
      { mcs::util::read::read<unsigned> (options.at (option::message_size))
      };
    auto const repetitions
      { mcs::util::read::read<unsigned> (options.at (option::repetitions))
      };
    auto const number_of_clients
      { mcs::util::read::read<unsigned>
          ( options.at (option::number_of_clients)
          )
      };

    return mcs::util::ASIO::run
      ( mcs::util::read::from_file<mcs::util::ASIO::AnyConnectable>
          (ping_service_path / "SERVER")
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (mcs::util::ASIO::Connectable<Protocol> connectable)
        {
          auto io_context
            { mcs::rpc::ScopedRunningIOContext
              { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
                  { mcs::util::read::read<unsigned>
                      ( options.at (option::number_of_threads)
                      )
                  }
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
              { std::chrono::duration_cast<Ticks> (end - start)
              };

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
  return mcs::util::main (argc, argv, help, client_main);
}

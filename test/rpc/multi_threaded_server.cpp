// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <chrono>
#include <fmt/chrono.h>
#include <future>
#include <gtest/gtest.h>
#include <iterator>
#include <map>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <thread>
#include <vector>

namespace
{
  using Ticks = std::chrono::microseconds;

  constexpr auto number_of_ticks_per_sleep {100u};
  constexpr auto number_of_sleeps_per_thread {1000u};
  constexpr auto number_of_threads {10u};

  struct Handler
  {
    struct Sleep { using Response = void; };
    auto operator() (Sleep) -> Sleep::Response
    {
      std::this_thread::sleep_for (Ticks {number_of_ticks_per_sleep});
    }
  };

  template<mcs::rpc::is_protocol Protocol>
    auto multi_sleep ( unsigned int server_threads
                     , unsigned int client_threads
                     )
  {
    using Dispatcher = mcs::rpc::Dispatcher<Handler, Handler::Sleep>;

    auto io_context_server
      { mcs::rpc::ScopedRunningIOContext
          {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {server_threads}}
      };
    auto io_context_client
      { mcs::rpc::ScopedRunningIOContext
          {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {client_threads}}
      };

    auto const provider
      { mcs::rpc::make_provider<Protocol, Dispatcher>
          ( typename Protocol::endpoint{}
          , io_context_server
          )
      };

    std::vector<std::future<void>> clients;

    for (auto i {0u}; i < number_of_threads; ++i)
    {
      clients.emplace_back
        ( std::async
          ( std::launch::async
          , [&, address = provider.local_endpoint()]
            {
              auto const client
                { mcs::rpc::make_client< Protocol
                                  , Dispatcher
                                  , mcs::rpc::access_policy::Exclusive
                                  >
                  ( io_context_client
                  , address
                  )
                };

              for (auto k {0u}; k < number_of_sleeps_per_thread; ++k)
              {
                client (Handler::Sleep{});
              }
            }
          )
        );
    }

    std::ranges::for_each
      ( clients
      , [] (auto& client)
        {
          client.get();
        }
      );
  }

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;
  template<class> struct RPCClientT : public ::testing::Test{};
  TYPED_TEST_SUITE (RPCClientT, Protocols);
}

namespace
{
  template<typename Range, typename Fun>
    auto for_each_ordered_pair (Range const& range, Fun fun)
  {
    for (auto x {std::cbegin (range)}; x != std::cend (range); ++x)
    {
      for (auto y {std::next (x)}; y != std::cend (range); ++y)
      {
        fun (*x, *y);
      }
    }
  }
}

TYPED_TEST (RPCClientT, multi_threaded_server)
{
  using Protocol = TypeParam;
  using Clock = std::chrono::steady_clock;

  auto numbers_of_server_threads {std::vector<unsigned int> {{1u, 2u, 4u}}};
  auto numbers_of_client_threads {std::vector<unsigned int> {{1u, 2u, 4u}}};
  auto ticks {std::map<unsigned int, std::map<unsigned int, Ticks>>{}};

  for (auto number_of_server_threads : numbers_of_server_threads)
  {
    for (auto number_of_client_threads : numbers_of_client_threads)
    {
      auto const start {Clock::now()};
      multi_sleep<Protocol>
        ( number_of_server_threads
        , number_of_client_threads
        );
      auto const end {Clock::now()};

      auto const duration {std::chrono::duration_cast<Ticks> (end - start)};

      ticks[number_of_server_threads][number_of_client_threads] = duration;

      fmt::print ("server_threads {} client_threads {} -> total time {}\n"
                 , number_of_server_threads
                 , number_of_client_threads
                 , duration
                 );
    }
  }

  // weak test: the real expectation is that the times not only go
  // down but go down _exponentially_
  for_each_ordered_pair
    ( numbers_of_server_threads
    , [&] (auto less, auto more)
      {
        for (auto [_l, time_less] : ticks.at (less))
        {
          for (auto [_m, time_more] : ticks.at (more))
          {
            ASSERT_LT (time_more, time_less);
          }
        }
      }
    );
}

// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <vector>

namespace
{
  struct Handler
  {
    struct Sleep { unsigned int mus; using Response = unsigned int; };

    auto operator() (Sleep sleep) -> asio::awaitable<Sleep::Response>
    {
      asio::steady_timer timer {co_await asio::this_coro::executor};
      timer.expires_after (std::chrono::microseconds (sleep.mus));
      co_await timer.async_wait (asio::use_awaitable);
      co_return sleep.mus;
    }
  };

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;
  template<class> struct RPCClientT : public mcs::testing::random::Test{};
  TYPED_TEST_SUITE (RPCClientT, Protocols);
}

TYPED_TEST ( RPCClientT
           , many_concurrent_requests_are_possible
           )
{
  using Protocol = TypeParam;
  using Dispatcher = mcs::rpc::Dispatcher<Handler, Handler::Sleep>;

  auto io_context_server
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };
  auto io_context_client
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };

  auto const provider
    { mcs::rpc::make_provider<Protocol, Dispatcher> ({}, io_context_server)
    };

  auto const client
    { mcs::rpc::make_client<Protocol, Dispatcher, mcs::rpc::access_policy::Concurrent>
        ( io_context_client
        , provider.local_endpoint()
        )
    };

  auto random_mus {mcs::testing::random::value<unsigned int> {0, 100}};

  std::vector< std::tuple< unsigned int
                         , std::future<Handler::Sleep::Response>
                         >
             > requests
    ;

  for (auto i {0}; i < 10000; ++i)
  {
    auto mus {random_mus()};
    requests.emplace_back (mus, client.get_future (Handler::Sleep {mus}));
  }

  for (auto& [mus, request] : requests)
  {
    ASSERT_EQ (request.get(), mus);
  }
}

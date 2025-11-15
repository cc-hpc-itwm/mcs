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

namespace
{
  struct Handler
  {
    struct Sleep { unsigned int ms; using Response = unsigned int; };
    struct Inc { int value; using Response = int; };

    auto operator() (Sleep sleep) const -> asio::awaitable<Sleep::Response>
    {
      asio::steady_timer timer {co_await asio::this_coro::executor};
      timer.expires_after (std::chrono::milliseconds (sleep.ms));
      co_await timer.async_wait (asio::use_awaitable);
      co_return sleep.ms;
    }
    auto operator() (Inc i) const noexcept -> Inc::Response
    {
      return++ i.value;
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
           , async_operation_can_be_dropped_and_client_still_works
           )
{
  using Protocol = TypeParam;
  using Dispatcher = mcs::rpc::Dispatcher
                     < Handler
                     , Handler::Sleep
                     , Handler::Inc
                     >
    ;
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

  // \note access policies Exclusive and Concurrent do not allow to
  // drop operations as there is the race between the destruction of
  // the operation-completion and the arrival of the result.
  auto const client
    { mcs::rpc::make_client<Protocol, Dispatcher, mcs::rpc::access_policy::Sequential>
        ( io_context_client
        , provider.local_endpoint()
        )
    };

  {
    auto sleep {client.get_future (Handler::Sleep {1000u})};
    ASSERT_EQ
      ( sleep.wait_for (std::chrono::milliseconds {100u})
      , std::future_status::timeout
      );
    // future is not yet ready and has timed out and client decides
    // to not wait for the result any longer but to drop the request
  }

  // client is still useable with synchronous and with asynchroonous
  // operations
  {
    auto const value {mcs::testing::random::value<int>{}()};
    ASSERT_EQ (client (Handler::Inc {value}), value + 1);
  }

  auto const ms {mcs::testing::random::value<unsigned int> {0, 1000}()};
  auto sleep {client.get_future (Handler::Sleep {ms})};
  ASSERT_EQ (sleep.get(), ms);
}

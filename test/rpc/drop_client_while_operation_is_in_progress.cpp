// Copyright (C) 2023-2025 Fraunhofer ITWM
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
#include <mcs/rpc/Concepts.hpp>
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

    auto operator() (Sleep sleep) const -> asio::awaitable<Sleep::Response>
    {
      asio::steady_timer timer {co_await asio::this_coro::executor};
      timer.expires_after (std::chrono::milliseconds (sleep.ms));
      co_await timer.async_wait (asio::use_awaitable);
      co_return sleep.ms;
    }
  };

  template<mcs::rpc::is_protocol P, mcs::rpc::is_access_policy AP>
    struct ProtocolAndAccessPolicy
  {
    using Protocol = P;
    using AccessPolicy = AP;
  };

  using ProtocolsAndAccessPolicies = ::testing::Types
    < ProtocolAndAccessPolicy<asio::ip::tcp               , mcs::rpc::access_policy::Exclusive>
    , ProtocolAndAccessPolicy<asio::local::stream_protocol, mcs::rpc::access_policy::Exclusive>

    , ProtocolAndAccessPolicy<asio::ip::tcp               , mcs::rpc::access_policy::Sequential>
    , ProtocolAndAccessPolicy<asio::local::stream_protocol, mcs::rpc::access_policy::Sequential>

    , ProtocolAndAccessPolicy<asio::ip::tcp               , mcs::rpc::access_policy::Concurrent>
    , ProtocolAndAccessPolicy<asio::local::stream_protocol, mcs::rpc::access_policy::Concurrent>
    >;
  template<class> struct RPCClientT : public mcs::testing::random::Test
  {
    mcs::testing::random::value<unsigned int> random_sleep_time {10u, 50u};
  };
  TYPED_TEST_SUITE (RPCClientT, ProtocolsAndAccessPolicies);
}

TYPED_TEST ( RPCClientT
           , client_can_be_dropped_while_async_operation_is_in_progress
           )
{
  using Protocol = typename TypeParam::Protocol;
  using AccessPolicy = typename TypeParam::AccessPolicy;
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

  auto const ms {this->random_sleep_time()};

  std::future<Handler::Sleep::Response> sleep;

  {
    auto const client
      { mcs::rpc::make_client<Protocol, Dispatcher, AccessPolicy>
        ( io_context_client
        , provider.local_endpoint()
        )
      };

    sleep = client.get_future (Handler::Sleep {ms});

    ASSERT_EQ
      ( sleep.wait_for (std::chrono::milliseconds {0u})
      , std::future_status::timeout
      );
    // future is not yet ready
  }
  // client is destructed

  // operation result can still be accessed
  ASSERT_EQ (sleep.get(), ms);
}

TYPED_TEST ( RPCClientT
           , temporary_client_can_be_used
           )
{
  using Protocol = typename TypeParam::Protocol;
  using AccessPolicy = typename TypeParam::AccessPolicy;
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

  auto const ms {this->random_sleep_time()};
  auto sleep
    { mcs::rpc::make_client<Protocol, Dispatcher, AccessPolicy>
        ( io_context_client
        , provider.local_endpoint()
        ).get_future (Handler::Sleep {ms})
    };

  ASSERT_EQ (sleep.get(), ms);
}

// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
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

namespace
{
  struct Handler
  {
    struct Sleep { unsigned int ms; using Response = unsigned int; };
    struct Nop { using Response = void; };
    struct NopAwaitable { using Response = void; };

    auto operator() (Sleep sleep) const -> asio::awaitable<Sleep::Response>
    {
      asio::steady_timer timer {co_await asio::this_coro::executor};
      timer.expires_after (std::chrono::milliseconds (sleep.ms));
      co_await timer.async_wait (asio::use_awaitable);
      co_return sleep.ms;
    }
    auto operator() (Nop) const noexcept -> Nop::Response
    {
      return;
    }
    auto operator() (NopAwaitable) const -> asio::awaitable<Nop::Response>
    {
      co_return;
    }
  };

  using Dispatcher = mcs::rpc::Dispatcher
                   < Handler
                   , Handler::Sleep
                   , Handler::Nop
                   , Handler::NopAwaitable
                   >
    ;

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;
  template<class> struct RPCClientT : public ::testing::Test{};
  TYPED_TEST_SUITE (RPCClientT, Protocols);

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
  template<class> struct RPCClientPA : public ::testing::Test{};
  TYPED_TEST_SUITE (RPCClientPA, ProtocolsAndAccessPolicies);
}

TYPED_TEST ( RPCClientPA
           , async_tasks_from_different_clients_can_overtake_each_other
           )
{
  using Protocol = typename TypeParam::Protocol;
  using AccessPolicy = typename TypeParam::AccessPolicy;

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

  auto const client_sleep
    { mcs::rpc::make_client<Protocol, Dispatcher, AccessPolicy>
        ( io_context_client
        , provider.local_endpoint()
        )
    };
  auto const client_nop
    { mcs::rpc::make_client<Protocol, Dispatcher, AccessPolicy>
        ( io_context_client
        , provider.local_endpoint()
        )
    };
  auto const client_nop_awaitable
    { mcs::rpc::make_client<Protocol, Dispatcher, AccessPolicy>
        ( io_context_client
        , provider.local_endpoint()
        )
    };

  auto const ms {100u};
  auto sleep {client_sleep.get_future (Handler::Sleep {ms})};

  client_nop (Handler::Nop{});
  client_nop_awaitable (Handler::NopAwaitable{});

  ASSERT_EQ
    ( sleep.wait_for (std::chrono::microseconds {0})
    , std::future_status::timeout
    );

  ASSERT_EQ (sleep.get(), ms);
}

TYPED_TEST ( RPCClientT
           , async_tasks_from_same_sequential_client_cannot_overtake_each_other
           )
{
  using Protocol = TypeParam;

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
    { mcs::rpc::make_client<Protocol, Dispatcher, mcs::rpc::access_policy::Sequential>
        ( io_context_client
        , provider.local_endpoint()
        )
    };

  auto const ms {100u};
  auto sleep {client.get_future (Handler::Sleep {ms})};

  client (Handler::Nop{});
  client (Handler::NopAwaitable{});

  ASSERT_EQ
    ( sleep.wait_for (std::chrono::microseconds {0})
    , std::future_status::ready
    );

  ASSERT_EQ (sleep.get(), ms);
}

TYPED_TEST ( RPCClientT
           , async_tasks_from_same_concurrent_client_can_overtake_each_other
           )
{
  using Protocol = TypeParam;

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

  auto const ms {100u};
  auto sleep {client.get_future (Handler::Sleep {ms})};

  client (Handler::Nop{});
  client (Handler::NopAwaitable{});

  ASSERT_EQ
    ( sleep.wait_for (std::chrono::microseconds {0})
    , std::future_status::timeout
    );

  ASSERT_EQ (sleep.get(), ms);
}

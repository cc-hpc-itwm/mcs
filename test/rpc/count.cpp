// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
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
#include <mcs/util/require_semi.hpp>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace
{
  auto micros()
  {
    static auto random_UL {mcs::testing::random::value<unsigned long> {0ul, 10ul}};

    return random_UL();
  }

  auto sleep()
  {
    std::this_thread::sleep_for (std::chrono::microseconds {micros()});
  }
  auto async_sleep() -> asio::awaitable<void>
  {
    auto timer {asio::steady_timer {co_await asio::this_coro::executor}};
    timer.expires_after (std::chrono::microseconds {micros()});
    co_await timer.async_wait (asio::use_awaitable);
  }

  struct SharedCounter
  {
    int value {0};
    std::mutex _guard;
  };

  struct Handler
  {
    Handler (SharedCounter* counter) : _counter {counter} {}
    SharedCounter* _counter;

    struct Inc { using Response = int; };
    auto operator() (Inc) const -> int
    {
      auto const old {_counter->value};
      sleep();
      return _counter->value = old + 1;
    }

    struct AsyncInc { using Response = int; };
    auto operator() (AsyncInc) const -> asio::awaitable<int>
    {
      auto const old {_counter->value};
      co_await async_sleep();
      co_return _counter->value = old + 1;
    }

    struct AtomicAsyncInc { using Response = int; };
    auto operator() (AtomicAsyncInc) const -> asio::awaitable<int>
    {
      co_await async_sleep();
      co_return _counter->value++;
    }

    struct SynchronizedInc { using Response = int; };
    auto operator() (SynchronizedInc) const -> int
    {
      auto const lock {std::lock_guard {_counter->_guard}};
      auto const old {_counter->value};
      sleep();
      return _counter->value = old + 1;
    }
  };

  constexpr auto number_of_threads {10u};
  constexpr auto number_of_increments_per_thread {1000u};

  template< mcs::rpc::is_protocol Protocol
          , mcs::rpc::is_access_policy AccessPolicy
          , typename Command
          >
    auto count ( unsigned int server_threads
               , auto&&... shared_state
               )
  {
    using Dispatcher = mcs::rpc::Dispatcher<Handler, Command>;

    auto io_context_server
      { mcs::rpc::ScopedRunningIOContext
          {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {server_threads}}
      };
    auto io_context_client
      { mcs::rpc::ScopedRunningIOContext
          {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
      };

    auto const provider
      { mcs::rpc::make_provider<Protocol, Dispatcher>
        ( typename Protocol::endpoint{}
        , io_context_server
        , std::forward<decltype (shared_state)> (shared_state)...
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
                { mcs::rpc::make_client<Protocol, Dispatcher, AccessPolicy>
                  ( io_context_client
                  , address
                  )
                };

              for (auto k {0u}; k < number_of_increments_per_thread; ++k)
              {
                client (Command{});
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
  template<class> struct RPCClientT : public mcs::testing::random::Test{};
  TYPED_TEST_SUITE (RPCClientT, ProtocolsAndAccessPolicies);
}

#define MCS_TEST_RPC_COUNT(_name, _command, _server_threads, _expectation) \
  TYPED_TEST (RPCClientT, count ## _name)                                  \
  {                                                                        \
    using Protocol = typename TypeParam::Protocol;                         \
    using AccessPolicy = typename TypeParam::AccessPolicy;                 \
                                                                           \
    SharedCounter counter;                                                 \
                                                                           \
    count<Protocol, AccessPolicy, Handler::_command>                       \
      (_server_threads, std::addressof (counter));                         \
                                                                           \
    _expectation                                                           \
      ( counter.value                                                      \
      , number_of_threads * number_of_increments_per_thread                \
      );                                                                   \
  } MCS_UTIL_REQUIRE_SEMI()
#define MCS_TEST_RPC_WORKS(_name, _command, _server_threads)               \
  MCS_TEST_RPC_COUNT (_name, _command, _server_threads, ASSERT_EQ)
#define MCS_TEST_RPC_FAILS(_name, _command, _server_threads)               \
  MCS_TEST_RPC_COUNT (_name, _command, _server_threads, ASSERT_LE)

MCS_TEST_RPC_WORKS (single_threaded_server_synchronized, SynchronizedInc, 1u);
MCS_TEST_RPC_WORKS (single_threaded_server_synchronous , Inc            , 1u);
MCS_TEST_RPC_WORKS (single_threaded_server_async_atomic, AtomicAsyncInc , 1u);
MCS_TEST_RPC_FAILS (single_threaded_server_async       , AsyncInc       , 1u);

MCS_TEST_RPC_WORKS (multi_threaded_server_synchronized, SynchronizedInc, 4u);
MCS_TEST_RPC_FAILS (multi_threaded_server_synchronous , Inc            , 4u);
MCS_TEST_RPC_FAILS (multi_threaded_server_async_atomic, AtomicAsyncInc , 4u);
MCS_TEST_RPC_FAILS (multi_threaded_server_async       , AsyncInc       , 4u);

#undef MCS_TEST_RPC_FAILS
#undef MCS_TEST_RPC_WORKS
#undef MCS_TEST_RPC_COUNT

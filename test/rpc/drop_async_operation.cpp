// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <asio/steady_timer.hpp>
#include <asio/this_coro.hpp>
#include <asio/use_awaitable.hpp>
#include <chrono>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <limits>
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
    struct Sleep
    {
      using Response = unsigned int;

      unsigned int ms;
    };

    struct Inc
    {
      using Response = int;

      int value;
    };

    auto operator() (Sleep sleep) const -> asio::awaitable<Sleep::Response>
    {
      auto timer {asio::steady_timer {co_await asio::this_coro::executor}};

      timer.expires_after (std::chrono::milliseconds {sleep.ms});

      co_await timer.async_wait (asio::use_awaitable);

      co_return sleep.ms;
    }

    auto operator() (Inc command) const noexcept -> Inc::Response
    {
      return ++command.value;
    }
  };

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;

  template<class>
    struct RPCClientT : public mcs::testing::random::Test{};

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
                     >;

  auto io_context_provider
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
        { std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
        }
      }
    };
  auto io_context_client
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
        { std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
        }
      }
    };

  auto const provider
    { mcs::rpc::make_provider<Protocol, Dispatcher>
      ( {}
      , io_context_provider
      )
    };

  // Dropping a future does not cancel its call. Sequential admission
  // must continue after the response even when nobody retrieves the
  // result.
  auto const client
    { mcs::rpc::make_client< Protocol
                           , Dispatcher
                           , mcs::rpc::access_policy::Sequential
                           >
      ( io_context_client
      , provider.local_endpoint()
      )
    };

  {
    auto const ms
      { std::invoke (mcs::testing::random::value<unsigned int> {1000u, 2000u})
      };
    auto sleep {client.get_future (Handler::Sleep {ms})};

    ASSERT_EQ
      ( sleep.wait_for (std::chrono::milliseconds {100u})
      , std::future_status::timeout
      );

    // future is not yet ready and has timed out and client decides
    // to not wait for the result any longer but to drop the request
  }

  // client is still usable with synchronous and with asynchronous
  // operations
  {
    auto const value
      { std::invoke
        ( mcs::testing::random::value<int>
          { std::numeric_limits<int>::lowest()
          , std::numeric_limits<int>::max() - 1
          }
        )
      };

    ASSERT_EQ (client (Handler::Inc {value}), value + 1);
  }

  {
    auto const ms
      { std::invoke (mcs::testing::random::value<unsigned int> {0u, 1000u})
      };
    auto sleep {client.get_future (Handler::Sleep {ms})};
    ASSERT_EQ (sleep.get(), ms);
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <future>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <vector>

namespace
{
  struct CallCounts
  {
    std::atomic<std::size_t> started {0};
    std::atomic<std::size_t> completed {0};
    std::atomic<std::size_t> errors {0};
  };

  struct CallObserver
  {
    mcs::util::not_null<CallCounts> counts;

    auto call_started() noexcept -> void
    {
      ++counts->started;
    }

    auto call_completed() noexcept -> void
    {
      ++counts->completed;
    }

    auto error_execution() noexcept -> void
    {
      ++counts->errors;
    }
  };

  struct BlockingHandler
  {
    static constexpr auto timeout {std::chrono::seconds {5}};

    struct Block
    {
      using Response = bool;
    };

    struct Release
    {
      using Response = void;
    };

    explicit BlockingHandler
      ( mcs::util::not_null<std::promise<void>> started
      , mcs::util::not_null<std::promise<void>> release
      , std::size_t number_of_requests
      )
        : _started {started}
        , _release {release}
        , _released {release->get_future()}
        , _remaining_requests {number_of_requests}
    {}

    auto operator() (Block) const -> Block::Response
    {
      if (--_remaining_requests == 0)
      {
        _started->set_value();
      }

      return _released.wait_for (timeout) == std::future_status::ready;
    }

    auto operator() (Release) const -> Release::Response
    {
      _release->set_value();
    }

  private:
    mcs::util::not_null<std::promise<void>> _started;
    mcs::util::not_null<std::promise<void>> _release;
    std::future<void> _released;
    mutable std::atomic<std::size_t> _remaining_requests;
  };

  using BlockingDispatcher = mcs::rpc::Dispatcher
                           < BlockingHandler
                           , BlockingHandler::Block
                           , BlockingHandler::Release
                           >;

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;

  template<class>
    struct SecondRequestWhileFirstHandlerBlocked : public ::testing::Test{};
  TYPED_TEST_SUITE (SecondRequestWhileFirstHandlerBlocked, Protocols);

  template<mcs::rpc::is_protocol Protocol>
    auto check_requests_while_handlers_are_blocked
      ( std::size_t number_of_requests
      , unsigned int client_threads
      ) -> void
  {
    auto started_promise {std::promise<void>{}};
    auto release_promise {std::promise<void>{}};
    auto counts {CallCounts{}};

    auto io_context_provider
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {4u}
        }
      };
    auto io_context_client
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {client_threads}
        }
      };
    auto const provider
      { mcs::rpc::make_provider<Protocol, BlockingDispatcher>
        ( {}
        , io_context_provider
        , std::addressof (started_promise)
        , std::addressof (release_promise)
        , number_of_requests
        )
      };
    auto const unobserved_client
      { mcs::rpc::make_client< Protocol
                             , BlockingDispatcher
                             , mcs::rpc::access_policy::Concurrent
                             >
        ( io_context_client
        , provider.local_endpoint()
        )
      };
    auto const client
      { unobserved_client.template observe<CallObserver>
        ( std::addressof (counts)
        )
      };

    auto blocks {std::vector<std::future<bool>>{}};
    blocks.reserve (number_of_requests);

    std::generate_n
      ( std::back_inserter (blocks)
      , number_of_requests
      , [&]
        {
          return client.get_future (BlockingHandler::Block{});
        }
      );

    auto started {started_promise.get_future()};

    ASSERT_EQ ( started.wait_for (BlockingHandler::timeout)
              , std::future_status::ready
              );

    auto release {client.get_future (BlockingHandler::Release{})};

    for (auto& block : blocks)
    {
      ASSERT_EQ ( block.wait_for (2 * BlockingHandler::timeout)
                , std::future_status::ready
                );
      EXPECT_TRUE (block.get());
    }

    ASSERT_EQ ( release.wait_for (BlockingHandler::timeout)
              , std::future_status::ready
              );

    release.get();
    EXPECT_EQ (counts.started.load(), number_of_requests + 1);
    EXPECT_EQ (counts.completed.load(), number_of_requests + 1);
    EXPECT_EQ (counts.errors.load(), 0u);
  }
}

TYPED_TEST ( SecondRequestWhileFirstHandlerBlocked
           , second_request_executes_while_first_handler_is_blocked
           )
{
  check_requests_while_handlers_are_blocked<TypeParam> (1u, 1u);
}

TYPED_TEST ( SecondRequestWhileFirstHandlerBlocked
           , release_executes_while_three_handlers_are_blocked_one_client_thread
           )
{
  check_requests_while_handlers_are_blocked<TypeParam> (3u, 1u);
}

TYPED_TEST ( SecondRequestWhileFirstHandlerBlocked
           , release_executes_while_three_handlers_are_blocked_four_client_threads
           )
{
  check_requests_while_handlers_are_blocked<TypeParam> (3u, 4u);
}

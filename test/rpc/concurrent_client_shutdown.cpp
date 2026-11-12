// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/awaitable.hpp>
#include <asio/buffer.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <asio/read.hpp>
#include <asio/steady_timer.hpp>
#include <asio/this_coro.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/error/Completion.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/serialization/error/Save.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

namespace
{
  constexpr auto timeout {std::chrono::seconds {5}};

  struct State
  {
    explicit State (std::size_t count)
      : remaining {count}
    {}

    std::atomic<std::size_t> remaining;
    std::promise<void> started;
  };

  struct Handler
  {
    struct Wait
    {
      using Response = void;
    };

    struct Echo
    {
      using Response = int;

      int value;
    };

    struct Stream
    {
      using Response = int;

      int value;

      template<typename Socket>
        auto stream (Socket& socket) const -> void
      {
        asio::write ( socket
                    , asio::buffer (std::addressof (value), sizeof (value))
                    );
      }
    };

    struct Large
    {
      using Response = void;

      explicit Large
        ( std::size_t size
        , std::shared_ptr<std::promise<void>> saving_ = {}
        , std::shared_ptr<std::atomic<bool>> destroyed_ = {}
        )
          : bytes (size)
          , saving {std::move (saving_)}
          , destroyed {std::move (destroyed_)}
      {}

      explicit Large (mcs::serialization::IArchive& archive)
        : bytes {mcs::serialization::load<std::vector<std::byte>> (archive)}
      {}

      Large (Large const&) = delete;
      Large (Large&&) noexcept = default;
      auto operator= (Large const&) -> Large& = delete;
      auto operator= (Large&&) -> Large& = delete;
      ~Large()
      {
        if (destroyed)
        {
          destroyed->store (true);
        }
      }

      auto save
        ( mcs::serialization::OArchive& archive
        ) const -> mcs::serialization::OArchive&
      {
        if (saving)
        {
          saving->set_value();
        }

        return mcs::serialization::save (archive, bytes);
      }

      std::vector<std::byte> bytes;
      std::shared_ptr<std::promise<void>> saving;
      std::shared_ptr<std::atomic<bool>> destroyed;
    };

    struct ThrowOnSave
    {
      using Response = void;

      explicit ThrowOnSave (int value_)
        : value {value_}
      {}

      explicit ThrowOnSave (mcs::serialization::IArchive& archive)
        : value {mcs::serialization::load<int> (archive)}
      {}

      auto save
        ( mcs::serialization::OArchive&
        ) const -> mcs::serialization::OArchive&
      {
        throw std::runtime_error {"intentional serialization failure"};
      }

      int value;
    };

    explicit Handler (std::shared_ptr<State> state)
      : _state {std::move (state)}
    {}

    auto operator() (Wait) const -> asio::awaitable<void>
    {
      auto timer
        { asio::steady_timer
          { co_await asio::this_coro::executor, std::chrono::hours {1}}
        };

      if (--_state->remaining == 0)
      {
        _state->started.set_value();
      }

      co_await timer.async_wait (asio::use_awaitable);
    }

    auto operator() (Echo command) const noexcept -> int
    {
      return command.value;
    }
    auto operator() (Large) const noexcept -> void
    {}

    template<typename Socket>
      auto operator() (Stream command, Socket& socket) const -> int
    {
      auto value {0};

      asio::read ( socket
                 , asio::buffer (std::addressof (value), sizeof (value))
                 );

      if (value != command.value)
      {
        throw std::runtime_error {"Incorrect streamed value"};
      }

      return value;
    }
    auto operator() (ThrowOnSave) const -> void
    {
      ADD_FAILURE() << "Command with throwing serialization reached handler";
    }

  private:
    std::shared_ptr<State> _state;
  };

  struct Observer
  {
    auto call_started() noexcept -> void {}
    auto call_completed() noexcept -> void {}
    auto error_execution() noexcept -> void {}
  };

  struct SendLifetimeObserver : Observer
  {
    explicit SendLifetimeObserver
      ( std::shared_ptr<std::atomic<bool>> destroyed_
      ) noexcept
        : destroyed {std::move (destroyed_)}
    {}

    auto error_execution() -> void
    {
      EXPECT_TRUE (destroyed->load())
        << "Call failure was delivered before its send frame was destroyed";
    }

    std::shared_ptr<std::atomic<bool>> destroyed;
  };

  using Dispatcher = mcs::rpc::Dispatcher
                   < Handler
                   , Handler::Wait
                   , Handler::Echo
                   , Handler::Stream
                   , Handler::Large
                   , Handler::ThrowOnSave
                   >;

  auto expect_cancelled (std::future<void>& response) -> void
  {
    ASSERT_EQ (response.wait_for (timeout), std::future_status::ready);

    mcs::testing::require_exception
      ( [&]
        {
          response.get();
        }
      , mcs::testing::Assert<mcs::rpc::error::Completion>
          { [] (auto const& error)
            {
              mcs::testing::require_exception
                ( [&]
                  {
                    std::rethrow_exception (error.reason());
                  }
                , mcs::testing::Assert<std::system_error>
                    { [] (auto const& reason)
                      {
                        EXPECT_EQ ( reason.code()
                                  , asio::error::operation_aborted
                                  );
                      }
                    }
                );
            }
          }
      );
  }

  template<mcs::rpc::is_protocol Protocol>
    auto check_shutdown
      ( unsigned int client_threads
      , std::size_t count
      , bool wait_until_started
      ) -> void
  {
    auto provider_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        }
      };
    auto client_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {client_threads}
        }
      };

    auto state {std::make_shared<State> (count)};

    auto const provider
      { mcs::rpc::make_provider<Protocol, Dispatcher>
        ( {}
        , provider_context
        , state
        )
      };
    auto const client
      { mcs::rpc::make_client< Protocol
                             , Dispatcher
                             , mcs::rpc::access_policy::Concurrent
                             >
          ( client_context
          , provider.local_endpoint()
          )
      };

    auto const copy {client};
    auto const observed {copy.template observe<Observer>()};

    auto responses {std::vector<std::future<void>>{}};
    responses.reserve (count);

    std::generate_n
      ( std::back_inserter (responses)
      , count
      , [&]
        {
          return observed.get_future (Handler::Wait{});
        }
      );

    if (wait_until_started)
    {
      auto started {state->started.get_future()};

      ASSERT_EQ (started.wait_for (timeout), std::future_status::ready);
    }

    auto stopped {copy.shutdown()};
    auto stopped_again {observed.shutdown()};

    ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
    stopped.get();

    ASSERT_EQ (stopped_again.wait_for (timeout), std::future_status::ready);
    stopped_again.get();

    for (auto& response : responses)
    {
      expect_cancelled (response);
    }

    {
      auto late_response {client.get_future (Handler::Wait{})};
      expect_cancelled (late_response);
    }

    auto const other
      { mcs::rpc::make_client< Protocol
                             , Dispatcher
                             , mcs::rpc::access_policy::Concurrent
                             >
          ( client_context
          , provider.local_endpoint()
          )
      };

    {
      auto const value {std::invoke (mcs::testing::random::value<int>{})};
      auto echo {other.get_future (Handler::Echo {value})};
      ASSERT_EQ (echo.wait_for (timeout), std::future_status::ready);
      EXPECT_EQ (echo.get(), value);
    }

    {
      auto other_stopped {other.shutdown()};
      ASSERT_EQ (other_stopped.wait_for (timeout), std::future_status::ready);
      other_stopped.get();
    }
  }

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;
  template<class>
    struct ConcurrentClientShutdown : public mcs::testing::random::Test{};
  TYPED_TEST_SUITE (ConcurrentClientShutdown, Protocols);
}

TYPED_TEST (ConcurrentClientShutdown, cancels_pending_reads_one_worker)
{
  check_shutdown<TypeParam> (1u, 8u, true);
}

TYPED_TEST (ConcurrentClientShutdown, cancels_pending_reads_four_workers)
{
  check_shutdown<TypeParam> (4u, 8u, true);
}

TYPED_TEST (ConcurrentClientShutdown, shuts_down_while_calls_are_being_sent)
{
  check_shutdown<TypeParam> (4u, 100u, false);
}

TYPED_TEST (ConcurrentClientShutdown, shuts_down_an_unused_connection)
{
  check_shutdown<TypeParam> (1u, 0u, false);
}

TYPED_TEST (ConcurrentClientShutdown, cancels_queued_large_writes)
{
  using Protocol = TypeParam;

  auto provider_context
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
      }
    };
  auto client_context
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {4u}
      }
    };

  auto const provider
    { mcs::rpc::make_provider<Protocol, Dispatcher>
      ( {}
      , provider_context
      , std::make_shared<State> (0u)
      )
    };

  auto destroyed {std::make_shared<std::atomic<bool>> (false)};

  auto const client
    { mcs::rpc::make_client< Protocol
                           , Dispatcher
                           , mcs::rpc::access_policy::Concurrent
                           >
      ( client_context
      , provider.local_endpoint()
      ).template observe<SendLifetimeObserver> (destroyed)
    };

  provider_context.stop();
  provider_context.join();

  auto const mib
    { std::invoke (mcs::testing::random::value<std::size_t> {2ul, 10ul})
    };
  auto responses {std::vector<std::future<void>>{}};
  responses.reserve (mib + 1);

  auto saving {std::make_shared<std::promise<void>>()};
  auto write_started {saving->get_future()};

  responses.emplace_back
    ( client.get_future (Handler::Large {mib << 20ul, saving, destroyed})
    );
  std::generate_n
    ( std::back_inserter (responses)
    , mib
    , [&]
      {
        return client.get_future (Handler::Large {1ul << 20ul});
      }
    );

  ASSERT_EQ (write_started.wait_for (timeout), std::future_status::ready);

  auto stopped {client.shutdown()};
  ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
  stopped.get();

  for (auto& response : responses)
  {
    expect_cancelled (response);
  }
}

TYPED_TEST ( ConcurrentClientShutdown
           , context_teardown_can_abandon_pending_reads
           )
{
  using Protocol = TypeParam;

  auto const number_of_pending_reads
    { std::invoke (mcs::testing::random::value<std::size_t> {1ul, 25ul})
    };

  auto responses {std::vector<std::future<void>>{}};
  responses.reserve (number_of_pending_reads);

  {
    auto provider_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        }
      };
    auto client_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
          { std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
          }
        }
      };

    auto state {std::make_shared<State> (number_of_pending_reads)};

    auto const provider
      { mcs::rpc::make_provider<Protocol, Dispatcher>
        ( {}
        , provider_context
        , state
        )
      };
    auto const client
      { mcs::rpc::make_client< Protocol
                             , Dispatcher
                             , mcs::rpc::access_policy::Concurrent
                             >
          ( client_context
          , provider.local_endpoint()
          )
      };

    std::generate_n
      ( std::back_inserter (responses)
      , number_of_pending_reads
      , [&]
        {
          return client.get_future (Handler::Wait{});
        }
      );

    auto started {state->started.get_future()};

    ASSERT_EQ (started.wait_for (timeout), std::future_status::ready);
  }

  for (auto& response : responses)
  {
    ASSERT_EQ (response.wait_for (timeout), std::future_status::ready);

    mcs::testing::require_exception
      ( [&]
        {
          response.get();
        }
      , mcs::testing::Assert<std::future_error>
          { [] (auto const& error)
            {
              EXPECT_EQ (error.code(), std::future_errc::broken_promise);
            }
          }
      );
  }
}

TYPED_TEST (ConcurrentClientShutdown, shutdown_races_with_concurrent_callers)
{
  using Protocol = TypeParam;

  auto provider_context
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
      }
    };
  auto const number_of_concurrent_callers
    { std::invoke (mcs::testing::random::value<unsigned> {2u, 10u})
    };
  auto client_context
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
        { number_of_concurrent_callers
        }
      }
    };

  auto const provider
    { mcs::rpc::make_provider<Protocol, Dispatcher>
      ( {}
      , provider_context
      , std::make_shared<State>
        ( std::invoke (mcs::testing::random::value<unsigned>{})
        )
      )
    };
  auto const client
    { mcs::rpc::make_client< Protocol
                           , Dispatcher
                           , mcs::rpc::access_policy::Concurrent
                           >
      ( client_context
      , provider.local_endpoint()
      )
    };

  auto go {std::promise<void>{}};
  auto ready {go.get_future().share()};

  auto callers
    { std::vector<std::future<std::vector<std::future<void>>>>{}
    };
  callers.reserve (number_of_concurrent_callers);

  auto number_of_wait_calls {std::vector<unsigned>{}};
  number_of_wait_calls.reserve (number_of_concurrent_callers);

  std::generate_n
    ( std::back_inserter (number_of_wait_calls)
    , number_of_concurrent_callers
    , mcs::testing::random::value<unsigned> {10u, 100u}
    );

  std::generate_n
    ( std::back_inserter (callers)
    , number_of_concurrent_callers
    , [&, i {std::size_t {0}}]() mutable
      {
        return std::async
          ( std::launch::async
            , [client, ready, n = number_of_wait_calls.at (i++)]
            {
              ready.wait();

              auto responses {std::vector<std::future<void>>{}};
              responses.reserve (n);

              std::generate_n
                ( std::back_inserter (responses)
                , n
                , [&]
                  {
                    return client.get_future (Handler::Wait{});
                  }
                );

              return responses;
            }
          );
      }
    );

  go.set_value();

  auto stopped {client.shutdown()};

  ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
  stopped.get();

  for (auto& caller : callers)
  {
    ASSERT_EQ (caller.wait_for (timeout), std::future_status::ready);

    auto responses {caller.get()};

    for (auto& response : responses)
    {
      expect_cancelled (response);
    }
  }
}

TYPED_TEST ( ConcurrentClientShutdown
           , serialization_failure_closes_and_drains_connection
           )
{
  using Protocol = TypeParam;

  auto provider_context
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
      }
    };
  auto client_context
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
        { std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
        }
      }
    };

  auto state {std::make_shared<State> (1u)};

  auto const provider
    { mcs::rpc::make_provider<Protocol, Dispatcher>
      ( {}
      , provider_context
      , state
      )
    };
  auto const client
    { mcs::rpc::make_client< Protocol
                           , Dispatcher
                           , mcs::rpc::access_policy::Concurrent
                           >
        ( client_context
        , provider.local_endpoint()
        )
    };

  auto pending {client.get_future (Handler::Wait{})};

  auto started {state->started.get_future()};
  ASSERT_EQ (started.wait_for (timeout), std::future_status::ready);

  auto const value {std::invoke (mcs::testing::random::value<int>{})};
  auto broken {client.get_future (Handler::ThrowOnSave {value})};

  for (auto* response : {std::addressof (pending), std::addressof (broken)})
  {
    ASSERT_EQ (response->wait_for (timeout), std::future_status::ready);

    mcs::testing::require_exception
      ( [&]
        {
          response->get();
        }
      , mcs::testing::Assert<mcs::rpc::error::Completion>
          { [] (auto const& error)
            {
              mcs::testing::require_exception
                ( [&]
                  {
                    std::rethrow_exception (error.reason());
                  }
                , mcs::testing::assert_type_or_derived_type
                    < mcs::serialization::error::Save
                    >()
                , mcs::testing::assert_type_and_what<std::runtime_error>
                    ("intentional serialization failure")
                );
            }
          }
      );
  }

  auto stopped {client.shutdown()};
  ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
  stopped.get();
}

TYPED_TEST (ConcurrentClientShutdown, synchronous_stream_hook_remains_usable)
{
  using Protocol = TypeParam;

  auto provider_context
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
      }
    };
  auto client_context
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
      }
    };

  auto const provider
    { mcs::rpc::make_provider<Protocol, Dispatcher>
      ( {}
      , provider_context
      , std::make_shared<State> (0u)
      )
    };
  auto const client
    { mcs::rpc::make_client< Protocol
                           , Dispatcher
                           , mcs::rpc::access_policy::Concurrent
                           >
      ( client_context
      , provider.local_endpoint()
      )
    };

  {
    auto const value {std::invoke (mcs::testing::random::value<int>{})};
    auto response {client.get_future (Handler::Stream {value})};
    ASSERT_EQ (response.wait_for (timeout), std::future_status::ready);
    EXPECT_EQ (response.get(), value);
  }

  auto stopped {client.shutdown()};
  ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
  stopped.get();
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/any_io_executor.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/error.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <asio/post.hpp>
#include <asio/redirect_error.hpp>
#include <asio/steady_timer.hpp>
#include <asio/strand.hpp>
#include <asio/use_awaitable.hpp>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/nonstd/scope.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedIOContext.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/syscall/_exit.hpp>
#include <mcs/testing/syscall/eventfd.hpp>
#include <mcs/testing/syscall/kill.hpp>
#include <mcs/testing/syscall/waitpid_retrying_on_interrupt.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/syscall/close.hpp>
#include <mcs/util/syscall/fork.hpp>
#include <mcs/util/syscall/read.hpp>
#include <mcs/util/syscall/write.hpp>
#include <memory>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
#include <system_error>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
  [[nodiscard]] auto child_wait_for
    ( pid_t child
    , std::chrono::seconds timeout
    ) -> int
  {
    auto status {int {0}};
    auto const deadline {std::chrono::steady_clock::now() + timeout};

  WAIT:
    if (  mcs::testing::syscall::waitpid_retrying_on_interrupt
            ( child
            , std::addressof (status)
            , WNOHANG
            )
       == child
       )
    {
      return status;
    }

    if (std::chrono::steady_clock::now() >= deadline)
    {
      throw std::runtime_error
        { "Timed out waiting for child process to exit"
        };
    }

    std::this_thread::yield();

    goto WAIT;
  }

  struct RequestsAwaitingClientExit
    : std::enable_shared_from_this<RequestsAwaitingClientExit>
  {
    template<typename Executor>
      [[nodiscard]] explicit RequestsAwaitingClientExit
        ( Executor& executor
        , std::size_t number_of_requests
        , unsigned int expected_value
        , std::chrono::seconds timeout
        )
          : _executor {asio::make_strand (executor)}
          , _remaining_requests {number_of_requests}
          , _request_results (number_of_requests)
          , _expected_value {expected_value}
          , _timeout {timeout}
    {}

    [[nodiscard]] auto wait_for_client_exit() -> asio::awaitable<void>
    {
      return asio::co_spawn
        ( _executor
        , wait_on_gate (shared_from_this())
        , asio::use_awaitable
        );
    }

    auto deliver_result (std::size_t request_index, unsigned int value) -> void
    {
      _request_results.at (request_index).set_value (value);
    }

    [[nodiscard]] auto wait_for_all_requests_to_be_started
      ( std::chrono::seconds timeout
      ) -> std::future_status
    {
      return _all_requests_started_future.wait_for (timeout);
    }

    auto open_gate() -> void
    {
      _gate.open();
    }

    [[nodiscard]] auto all_results_are_delivered_and_correct() && -> bool
    {
      return std::ranges::all_of
        ( _request_results
        , [this, request_index {std::size_t {0}}] (auto& promise) mutable
          {
            auto const index {request_index++};

            auto result {promise.get_future()};

            if (result.wait_for (_timeout) != std::future_status::ready)
            {
              mcs::util::FMT::print_noexcept
                ( stderr
                , "Request {}: result not delivered within {} seconds\n"
                , index
                , _timeout.count()
                );

              return false;
            }

            if ( auto const value {result.get()}
               ; value != _expected_value
               )
            {
              mcs::util::FMT::print_noexcept
                ( stderr
                , "Request {}: expected result {}, got {}\n"
                , index
                , _expected_value
                , value
                );

              return false;
            }

            return true;
          }
        );
    }

  private:
    [[nodiscard]] static auto wait_on_gate
      ( std::shared_ptr<RequestsAwaitingClientExit> requests
      ) -> asio::awaitable<void>
    {
      if (--requests->_remaining_requests == 0)
      {
        requests->_all_requests_started.set_value();
      }

      co_await requests->_gate.wait();
    }

    struct async_gate
    {
      using Clock = std::chrono::steady_clock;

      [[nodiscard]] explicit async_gate (asio::any_io_executor executor)
          : _timer {std::move (executor), Clock::time_point::max()}
      {}

      [[nodiscard]] auto wait() -> asio::awaitable<void>
      {
        auto error {asio::error_code{}};

        co_await _timer.async_wait
          ( asio::redirect_error (asio::use_awaitable, error)
          );

        if (error && error != asio::error::operation_aborted)
        {
          throw std::system_error {error, "Waiting for client-exit gate"};
        }
      }

      auto open() -> void
      {
        asio::post
          ( _timer.get_executor()
          , [this]
            {
              std::ignore = _timer.expires_at (Clock::now());
            }
          );
      }

    private:
      asio::steady_timer _timer;
    };

    asio::strand<asio::any_io_executor> _executor;
    async_gate _gate {_executor};
    std::size_t _remaining_requests;
    std::promise<void> _all_requests_started;
    std::future<void> _all_requests_started_future
      { _all_requests_started.get_future()
      };
    std::vector<std::promise<unsigned int>> _request_results;
    unsigned int _expected_value;
    std::chrono::seconds _timeout;
  };

  struct Handler
  {
    struct WaitForClientExit
    {
      using Response = unsigned int;

      std::size_t request_index;
      bool throw_after_resume;
    };

    explicit Handler
      ( std::shared_ptr<RequestsAwaitingClientExit> requests
      , mcs::util::not_null<std::promise<void>> handler_destroyed_promise
      , mcs::util::not_null<unsigned int const> argument_value
      , unsigned int handler_value
      ) noexcept
        : _requests {std::move (requests)}
        , _handler_destroyed_promise {handler_destroyed_promise}
        , _argument_value {argument_value}
        , _handler_value {handler_value}
    {}

    Handler (Handler const&) = delete;
    Handler (Handler&&) = delete;
    auto operator= (Handler const&) -> Handler& = delete;
    auto operator= (Handler&&) -> Handler& = delete;

    ~Handler()
    {
      _handler_destroyed_promise->set_value();
    }

    auto operator()
      ( WaitForClientExit request
      ) const -> asio::awaitable<WaitForClientExit::Response>
    {
      co_await _requests->wait_for_client_exit();

      auto const result {_handler_value + *_argument_value};
      _requests->deliver_result (request.request_index, result);

      if (request.throw_after_resume)
      {
        throw std::runtime_error {"handler failure after client exit"};
      }

      co_return result;
    }

  private:
    std::shared_ptr<RequestsAwaitingClientExit> _requests;
    mcs::util::not_null<std::promise<void>> _handler_destroyed_promise;
    mcs::util::not_null<unsigned int const> _argument_value;
    unsigned int _handler_value;
  };

  template<mcs::rpc::is_protocol Protocol>
    auto check_client_process_exit
      ( unsigned int number_of_threads
      , std::size_t number_of_requests
      ) -> void
  {
    using Dispatcher = mcs::rpc::Dispatcher
                     < Handler
                     , Handler::WaitForClientExit
                     >;

    constexpr auto timeout {std::chrono::seconds {5}};

    auto random_value {mcs::testing::random::value<unsigned int>{}};
    auto const argument_value {std::invoke (random_value)};
    auto const handler_value {std::invoke (random_value)};
    auto const expected_value {argument_value + handler_value};
    auto const client_threads
      { std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
      };

    SCOPED_TRACE
      ( ::testing::Message{}
          << "threads: " << number_of_threads
          << ", client threads: " << client_threads
          << ", requests: " << number_of_requests
          << ", argument: " << argument_value
          << ", handler: " << handler_value
      );

    auto handler_destroyed_promise {std::promise<void>{}};

    auto io_context_provider
      { mcs::rpc::ScopedIOContext
        { mcs::rpc::ScopedIOContext::NumberOfThreads {number_of_threads}
        }
      };

    auto requests
      { std::make_shared<RequestsAwaitingClientExit>
        ( io_context_provider
        , number_of_requests
        , expected_value
        , timeout
        )
      };

    auto const provider
      { mcs::rpc::make_provider<Protocol, Dispatcher>
        ( {}
        , io_context_provider
        , requests
        , std::addressof (handler_destroyed_promise)
        , std::addressof (argument_value)
        , handler_value
        )
      };

    auto const stop_client {mcs::testing::syscall::eventfd (0u, 0)};

    auto const close_stop_client
      { mcs::nonstd::make_scope_exit
          ( [stop_client]() noexcept
            {
              try
              {
                mcs::util::syscall::close (stop_client);
              }
              catch (...)
              {
                mcs::util::FMT::print_noexcept
                  ( stderr
                  , "Closing client-stop eventfd failed: {}\n"
                  , std::current_exception()
                  );
              }
            }
          )
      };

    io_context_provider.notify_fork (asio::io_context::fork_prepare);

    auto const child
      { std::invoke
        ( [&]
          {
            try
            {
              return mcs::util::syscall::fork();
            }
            catch (...)
            {
              io_context_provider.notify_fork (asio::io_context::fork_parent);

              throw;
            }
          }
        )
      };

    if (child == 0)
    {
      try
      {
        io_context_provider.notify_fork (asio::io_context::fork_child);

        auto io_context_client
          { mcs::rpc::ScopedRunningIOContext
            { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {client_threads}
            }
          };

        auto const client
          { mcs::rpc::make_client< Protocol
                                , Dispatcher
                                , mcs::rpc::access_policy::Concurrent
                                >
            ( io_context_client
            , provider.local_endpoint()
            )
          };

        auto responses {std::vector<std::future<unsigned int>>{}};

        // Catch before unwinding responses: destruction can block
        //
        try
        {
          responses.reserve (number_of_requests);

          std::generate_n
            ( std::back_inserter (responses)
            , number_of_requests
            , [&client, request {std::size_t {0}}]() mutable
              {
                auto const index {request++};

                return std::async
                  ( std::launch::async
                  , [&client, index]
                    {
                      return client
                        ( Handler::WaitForClientExit
                          { .request_index = index
                          , .throw_after_resume = index % 2 != 0
                          }
                        );
                    }
                  );
              }
            );

          auto stop {std::uint64_t {0}};
          if ( auto const bytes_read
                 { mcs::util::syscall::read
                     ( stop_client
                     , std::addressof (stop)
                     , sizeof (stop)
                     )
                 }
             ; bytes_read != sizeof (stop) || stop != 1u
             )
          {
            mcs::util::FMT::print_noexcept
              ( stderr
              , "Invalid client-stop event: read {} bytes, value {}\n"
              , bytes_read
              , stop
              );

            mcs::testing::syscall::_exit (EXIT_FAILURE);
          }
        }
        catch (...)
        {
          mcs::util::FMT::print_noexcept
            ( stderr
            , "Client request setup or stop wait failed: {}\n"
            , std::current_exception()
            );

          mcs::testing::syscall::_exit (EXIT_FAILURE);
        }

        mcs::testing::syscall::_exit (EXIT_SUCCESS);
      }
      catch (...)
      {
        mcs::util::FMT::print_noexcept
          ( stderr
          , "Client process setup failed: {}\n"
          , std::current_exception()
          );

        mcs::testing::syscall::_exit (EXIT_FAILURE);
      }
    }

    io_context_provider.notify_fork (asio::io_context::fork_parent);

    ASSERT_GT (child, 0);

    auto cleanup_child
      { mcs::nonstd::make_scope_exit
          ( [child]() noexcept
            {
              try
              {
                mcs::testing::syscall::kill (child, SIGKILL);
              }
              catch (...)
              {
                mcs::util::FMT::print_noexcept
                  ( stderr
                  , "Killing client process during cleanup failed: {}\n"
                  , std::current_exception()
                  );
              }

              try
              {
                std::ignore = mcs::testing::syscall::waitpid_retrying_on_interrupt
                  ( child
                  , nullptr
                  , 0
                  );
              }
              catch (...)
              {
                mcs::util::FMT::print_noexcept
                  ( stderr
                  , "Reaping client process during cleanup failed: {}\n"
                  , std::current_exception()
                  );
              }
            }
          )
      };

    io_context_provider.start();

    ASSERT_EQ
      ( requests->wait_for_all_requests_to_be_started (timeout)
      , std::future_status::ready
      );

    {
      auto const stop {std::uint64_t {1}};
      ASSERT_EQ
        ( mcs::util::syscall::write
          ( stop_client
          , std::addressof (stop)
          , sizeof (stop)
          )
        , sizeof (stop)
        );
    }

    auto const status {child_wait_for (child, timeout)};
    cleanup_child.release();

    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), EXIT_SUCCESS);

    auto handler_destroyed {handler_destroyed_promise.get_future()};

    ASSERT_EQ
      ( handler_destroyed.wait_for (std::chrono::seconds {0})
      , std::future_status::timeout
      );

    requests->open_gate();

    EXPECT_TRUE (std::move (*requests).all_results_are_delivered_and_correct());

    ASSERT_EQ ( handler_destroyed.wait_for (timeout)
              , std::future_status::ready
              );
  }

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;

  template<class>
    struct HandlerAfterClientProcessExit : public mcs::testing::random::Test{};

  TYPED_TEST_SUITE (HandlerAfterClientProcessExit, Protocols);
}

TYPED_TEST ( HandlerAfterClientProcessExit
           , client_process_exit_keeps_handler_alive_one_thread_one_request
           )
{
  check_client_process_exit<TypeParam> (1u, 1u);
}

TYPED_TEST ( HandlerAfterClientProcessExit
           , client_process_exit_keeps_handler_alive_one_thread_three_requests
           )
{
  check_client_process_exit<TypeParam> (1u, 3u);
}

TYPED_TEST ( HandlerAfterClientProcessExit
           , client_process_exit_keeps_handler_alive_four_threads_one_request
           )
{
  check_client_process_exit<TypeParam> (4u, 1u);
}

TYPED_TEST ( HandlerAfterClientProcessExit
           , client_process_exit_keeps_handler_alive_four_threads_three_requests
           )
{
  check_client_process_exit<TypeParam> (4u, 3u);
}

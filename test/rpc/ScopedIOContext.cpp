// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/post.hpp>
#include <chrono>
#include <csignal>
#include <fmt/format.h>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <mcs/nonstd/scope.hpp>
#include <mcs/rpc/ScopedIOContext.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

namespace mcs::rpc
{
  static_assert
    ( std::is_same_v< ScopedIOContext::NumberOfThreads
                    , ScopedRunningIOContext::NumberOfThreads
                    >
    );

  namespace
  {
    constexpr auto enough_time_to_process {std::chrono::seconds {5}};
    constexpr auto short_time_for_potential_async_operation
      { std::chrono::milliseconds {10}
      };

    struct ScopedIOContextR : public testing::random::Test
    {
      unsigned int number_of_threads
        { std::invoke
          ( []
            {
              using RandomValue = testing::random::value<unsigned int>;
              return std::invoke (RandomValue {RandomValue::Min {1u}});
            }
          )
        };
    };
  }

  TEST (ScopedIOContext, zero_threads_throws)
  {
    testing::require_exception
      ( []
        {
          std::ignore = ScopedIOContext::NumberOfThreads {0u};
        }
      , testing::assert_type_and_what
          < ScopedIOContext::NumberOfThreads::Error::MustBePositive
          > ( "Number of threads must be positive."
            )
      );
  }

  TEST_F (ScopedIOContextR, thread_count_uses_canonical_spelling)
  {
    EXPECT_EQ
      ( fmt::format
        ( "{}"
        , ScopedIOContext::NumberOfThreads {number_of_threads}
        )
      , fmt::format
        ( "ScopedIOContext::NumberOfThreads {}"
        , std::make_tuple (number_of_threads)
        )
      );
  }

  TEST_F (ScopedIOContextR, eager_thread_count_uses_canonical_spelling)
  {
    EXPECT_EQ
      ( fmt::format
        ( "{}"
        , ScopedRunningIOContext::NumberOfThreads {number_of_threads}
        )
      , fmt::format
        ( "ScopedIOContext::NumberOfThreads {}"
        , std::make_tuple (number_of_threads)
        )
      );
  }

  TEST_F (ScopedIOContextR, thread_count_parses_canonical_spelling)
  {
    auto const parsed
      { util::read::read<ScopedIOContext::NumberOfThreads>
          ( fmt::format
            ( "ScopedIOContext::NumberOfThreads {}"
            , std::make_tuple (number_of_threads)
            )
          )
      };
    EXPECT_EQ (parsed.value, number_of_threads);
  }

  TEST_F (ScopedIOContextR, thread_count_format_round_trip)
  {
    testing::read_of_fmt_is_identity
      ( ScopedIOContext::NumberOfThreads {number_of_threads}
      , [&] (auto const& read) noexcept
        {
          return read.value == number_of_threads;
        }
      );
  }

  TEST (ScopedIOContext, posted_work_waits_for_start)
  {
    auto completed {std::promise<void>{}};

    auto context {ScopedIOContext {ScopedIOContext::NumberOfThreads {1u}}};

    asio::post (context, [&] { completed.set_value(); });

    auto ready {completed.get_future()};

    ASSERT_EQ
      ( ready.wait_for (short_time_for_potential_async_operation)
      , std::future_status::timeout
      );

    context.start();

    ASSERT_EQ ( ready.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    ready.get();
  }

  TEST_F (ScopedIOContextR, start_keeps_idle_context_running)
  {
    auto promises
      { std::vector<std::promise<void>>
        ( std::invoke (testing::random::value<unsigned> {2u, 10u})
        )
      };

    auto context {ScopedIOContext {ScopedIOContext::NumberOfThreads {1u}}};
    context.start();

    for (auto& completed : promises)
    {
      asio::post (context, [&] { completed.set_value(); });

      auto ready {completed.get_future()};

      ASSERT_EQ ( ready.wait_for (enough_time_to_process)
                , std::future_status::ready
                );

      ready.get();

      // Increase the probability that the execution thread goes idle
      //
      std::this_thread::sleep_for (short_time_for_potential_async_operation);
    }
  }

  TEST (ScopedIOContext, start_twice_throws)
  {
    auto context {ScopedIOContext {ScopedIOContext::NumberOfThreads {1u}}};
    context.start();

    testing::require_exception
      ( [&]
        {
          context.start();
        }
      , testing::assert_type_and_what<std::logic_error>
          ( "ScopedIOContext::start() may only be called once"
          )
      );
  }

  TEST (ScopedIOContext, join_before_start_is_a_no_op)
  {
    auto completed {std::promise<void>{}};

    auto context {ScopedIOContext {ScopedIOContext::NumberOfThreads {1u}}};
    context.join();

    asio::post (context, [&] { completed.set_value(); });

    context.start();

    auto ready {completed.get_future()};

    ASSERT_EQ ( ready.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    ready.get();
  }

  TEST (ScopedIOContext, join_does_not_stop_context)
  {
    auto context {ScopedIOContext {ScopedIOContext::NumberOfThreads {1u}}};
    context.start();

    auto const stop_context
      { nonstd::make_scope_exit ([&]() noexcept { context.stop(); })
      };

    auto joined
      { std::async
        ( std::launch::async
        , [&]() noexcept { context.join(); }
        )
      };

    EXPECT_EQ ( joined.wait_for (short_time_for_potential_async_operation)
              , std::future_status::timeout
              );
    EXPECT_FALSE (context.stopped());

    context.stop();

    ASSERT_EQ ( joined.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    joined.get();
  }

  TEST (ScopedIOContext, destruction_without_start_discards_posted_work)
  {
    auto ready
      { std::invoke
        ( [&]
          {
            auto _context
              { ScopedIOContext {ScopedIOContext::NumberOfThreads {1u}}
              };
            auto _completed {std::promise<void>{}};
            auto _ready {_completed.get_future()};
            asio::post
              ( _context
              , [completed = std::move (_completed)]() mutable
                {
                  completed.set_value();
                }
              );
            return _ready;
          }
        )
      };

    ASSERT_EQ ( ready.wait_for (short_time_for_potential_async_operation)
              , std::future_status::ready
              );

    testing::require_exception
      ( [&]
        {
          ready.get();
        }
      , testing::assert_type<std::future_error>()
      );
  }

  TEST (ScopedIOContext, destruction_joins_running_workers)
  {
    auto completed {std::promise<void>{}};
    auto ready {completed.get_future().share()};

    auto release {std::promise<void>{}};
    auto released {release.get_future().share()};

    auto release_worker
      { nonstd::make_scope_exit ([&]() noexcept { release.set_value(); })
      };

    auto context_destroyed
      { std::async
        ( std::launch::async
        , [&, ready, released]
          {
            auto context
              { ScopedIOContext {ScopedIOContext::NumberOfThreads {1u}}
              };
            asio::post
              ( context
              , [&, released]
                {
                  completed.set_value();
                  released.wait();
                }
              );
            context.start();

            if (  ready.wait_for (enough_time_to_process)
               != std::future_status::ready
               )
            {
              throw std::runtime_error {"ScopedIOContext worker did not start"};
            }
          }
        )
      };

    // The asynchronous execution is ongoing...
    //
    ASSERT_EQ ( ready.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    // ...and destruction waits for it to finish...
    //
    EXPECT_EQ
      ( context_destroyed.wait_for (short_time_for_potential_async_operation)
      , std::future_status::timeout
      );

    // ...which happens now...
    //
    release.set_value();
    release_worker.release();

    // ...and causes destruction to proceed.
    //
    ASSERT_EQ ( context_destroyed.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    context_destroyed.get();
  }

  TEST (ScopedIOContext, eager_wrapper_starts_automatically)
  {
    auto completed {std::promise<void>{}};

    auto context
      { ScopedRunningIOContext {ScopedRunningIOContext::NumberOfThreads {1u}}
      };

    asio::post (context, [&completed] { completed.set_value(); });

    auto ready {completed.get_future()};

    ASSERT_EQ ( ready.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    ready.get();
  }

  TEST (ScopedIOContext, configured_signal_stops_workers)
  {
    auto completed {std::promise<void>{}};

    auto context
      { ScopedIOContext {ScopedIOContext::NumberOfThreads {1u}, SIGUSR1}
      };

    asio::post (context, [&completed] { completed.set_value(); });

    context.start();

    auto ready {completed.get_future()};

    ASSERT_EQ ( ready.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    auto joined {std::future<void>{}};
    auto const stop_context
      { nonstd::make_scope_exit ([&]() noexcept  { context.stop(); })
      };

    ASSERT_EQ (std::raise (SIGUSR1), 0);

    joined = std::async
      ( std::launch::async
      , [&]() noexcept { context.join(); }
      );

    ASSERT_EQ ( joined.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    joined.get();

    EXPECT_TRUE (context.stopped());
  }

  TEST (ScopedIOContext, worker_exception_is_caught)
  {
    auto completed {std::promise<void>{}};

    auto context {ScopedIOContext {ScopedIOContext::NumberOfThreads {1u}}};

    asio::post
      ( context
      , [&]
        {
          completed.set_value();

          throw std::runtime_error {"expected worker failure"};
        }
      );

    context.start();

    auto ready {completed.get_future()};

    ASSERT_EQ ( ready.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    context.stop();

    auto const stop_context
      { nonstd::make_scope_exit
          ( [&context]() noexcept
            {
              context.stop();
            }
          )
      };
    auto joined
      { std::async
        ( std::launch::async
        , [&]() noexcept { context.join(); }
        )
      };

    ASSERT_EQ ( joined.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    joined.get();
  }

  TEST (ScopedIOContext, worker_exception_does_not_prevent_subsequent_work)
  {
    auto first_started {std::promise<void>{}};
    auto completed {std::promise<void>{}};

    auto context {ScopedIOContext {ScopedIOContext::NumberOfThreads {1u}}};

    asio::post
      ( context
      , [&first_started]
        {
          first_started.set_value();

          throw std::runtime_error {"expected worker failure"};
        }
      );

    context.start();

    auto started {first_started.get_future()};

    ASSERT_EQ ( started.wait_for (enough_time_to_process)
              , std::future_status::ready
              );

    started.get();

    asio::post (context, [&completed] { completed.set_value(); });

    auto ready {completed.get_future()};

    EXPECT_EQ ( ready.wait_for (enough_time_to_process)
              , std::future_status::ready
              );
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
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
#include <asio/use_future.hpp>
#include <atomic>
#include <chrono>
#include <concepts>
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
#include <mcs/rpc/detail/Buffer.hpp>
#include <mcs/rpc/detail/CallID.hpp>
#include <mcs/rpc/detail/CommandIndex.hpp>
#include <mcs/rpc/detail/Completion.hpp>
#include <mcs/rpc/detail/ReceivedFrame.hpp>
#include <mcs/rpc/detail/receive_buffer_with_header.hpp>
#include <mcs/rpc/error/Completion.hpp>
#include <mcs/rpc/error/HandlerException.hpp>
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
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
  constexpr auto timeout {std::chrono::seconds {5}};

  struct State
  {
    explicit State (asio::any_io_executor executor)
      : strand {asio::make_strand (std::move (executor))}
      , gate {strand, std::chrono::steady_clock::time_point::max()}
    {}

    asio::strand<asio::any_io_executor> strand;
    asio::steady_timer gate;
    std::promise<void> started;
    bool first {true};
  };

  [[nodiscard]] auto wait_on_gate
    ( std::shared_ptr<State> state
    ) -> asio::awaitable<void>
  {
    if (std::exchange (state->first, false))
    {
      state->started.set_value();
    }

    auto error {asio::error_code{}};

    co_await state->gate.async_wait
      ( asio::redirect_error (asio::use_awaitable, error)
      );

    if (error && error != asio::error::operation_aborted)
    {
      throw std::system_error {error, "Waiting for test gate"};
    }
  }

  struct Handler
  {
    struct Echo
    {
      using Response = int;

      int value;
    };

    struct Wait
    {
      using Response = int;
    };

    struct Throw
    {
      using Response = int;
    };

    struct ThrowOnStream
    {
      using Response = int;

      template<typename Socket>
        auto stream (Socket&) const -> void
      {
        throw std::runtime_error {"stream failure"};
      }
    };

    struct Large
    {
      using Response = int;

      explicit Large
        ( std::size_t size
        , std::shared_ptr<std::promise<void>> saving_
        , std::shared_ptr<std::atomic<bool>> destroyed_
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
        saving->set_value();

        return mcs::serialization::save (archive, bytes);
      }

      std::vector<std::byte> bytes;
      std::shared_ptr<std::promise<void>> saving;
      std::shared_ptr<std::atomic<bool>> destroyed;
    };

    struct ThrowOnSave
    {
      using Response = int;

      explicit ThrowOnSave (int value_)
        : value {value_}
      {
        if (value < 0)
        {
          throw std::runtime_error {"construction failure"};
        }

      }
      explicit ThrowOnSave (mcs::serialization::IArchive& archive)
        : value {mcs::serialization::load<int> (archive)}
      {}

      auto save
        ( mcs::serialization::OArchive&
        ) const -> mcs::serialization::OArchive&
      {
        throw std::runtime_error {"serialization failure"};
      }

      int value;
    };

    struct Broken
    {
      using Response = int;

      explicit Broken (int value_)
        : value {value_}
      {}

      explicit Broken (mcs::serialization::IArchive& archive)
        : value {mcs::serialization::load<int> (archive)}
      {}

      auto save
        ( mcs::serialization::OArchive& archive
        ) const noexcept -> mcs::serialization::OArchive&
      {
        return archive;
      }

      int value;
    };

    explicit Handler (std::shared_ptr<State> state) noexcept
      : _state {std::move (state)}
    {}

    auto operator() (Echo command) const noexcept -> int
    {
      return command.value;
    }

    auto operator() (Throw) const -> int
    {
      throw std::runtime_error {"handler failure"};
    }

    auto operator() (ThrowOnStream) const noexcept -> int
    {
      return 0;
    }

    auto operator() (Large) const noexcept -> int
    {
      return 0;
    }

    auto operator() (Wait) const -> asio::awaitable<int>
    {
      co_await asio::co_spawn
        ( _state->strand
        , wait_on_gate (_state)
        , asio::use_awaitable
        );

      co_return 42;
    }

    auto operator() (ThrowOnSave) const -> int
    {
      ADD_FAILURE() << "Throwing save reached handler";

      return 0;
    }

    auto operator() (Broken) const -> int
    {
      ADD_FAILURE() << "Broken archive reached handler";

      return 0;
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

  struct LifetimeObserver : Observer
  {
    explicit LifetimeObserver
      ( std::shared_ptr<std::atomic<bool>> destroyed_
      ) noexcept
        : destroyed {std::move (destroyed_)}
    {}

    auto error_execution() -> void
    {
      EXPECT_TRUE (destroyed->load())
        << "Call completed before its write released command buffers";
    }

    std::shared_ptr<std::atomic<bool>> destroyed;
  };

  struct CountingObserver : Observer
  {
    explicit CountingObserver
      ( std::shared_ptr<std::atomic<unsigned int>> completed_
      ) noexcept
        : completed {std::move (completed_)}
    {}

    auto call_completed() noexcept -> void
    {
      ++*completed;
    }

    std::shared_ptr<std::atomic<unsigned int>> completed;
  };

  template<typename Protocol_, typename Policy_>
    struct Configuration
  {
    using Protocol = Protocol_;
    using Policy = Policy_;
  };

  using Configurations = ::testing::Types
    < Configuration<asio::ip::tcp               , mcs::rpc::access_policy::Exclusive>
    , Configuration<asio::ip::tcp               , mcs::rpc::access_policy::Sequential>
    , Configuration<asio::ip::tcp               , mcs::rpc::access_policy::Concurrent>
    , Configuration<asio::local::stream_protocol, mcs::rpc::access_policy::Exclusive>
    , Configuration<asio::local::stream_protocol, mcs::rpc::access_policy::Sequential>
    , Configuration<asio::local::stream_protocol, mcs::rpc::access_policy::Concurrent>
    >;

  template<typename Configuration>
    struct UnifiedClientConnection : public mcs::testing::random::Test
  {
    using Policy = typename Configuration::Policy;
    using Protocol = typename Configuration::Protocol;

    template<typename Test>
      auto with_client
        ( unsigned int workers
        , Test test
        , bool stop_provider = false
        ) -> void
    {
      using Dispatcher = mcs::rpc::Dispatcher
                       < Handler
                       , Handler::Echo
                       , Handler::Wait
                       , Handler::ThrowOnSave
                       , Handler::Broken
                       , Handler::Large
                       , Handler::ThrowOnStream
                       , Handler::Throw
                       >;

      auto provider_context
        { mcs::rpc::ScopedRunningIOContext
          { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
            { std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
            }
          }
        };
      auto client_context
        { mcs::rpc::ScopedRunningIOContext
          { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {workers}
          }
        };

      auto state {std::make_shared<State> (provider_context.get_executor())};

      auto const provider
        { mcs::rpc::make_provider<Protocol, Dispatcher>
          ( {}
          , provider_context
          , state
          )
        };
      auto const client
        { mcs::rpc::make_client<Protocol, Dispatcher, Policy>
          ( client_context
          , provider.local_endpoint()
          ).template observe<Observer>()
        };

      if (stop_provider)
      {
        provider_context.stop();
        provider_context.join();
      }

      test (client, state);

      auto stopped {client.shutdown()};
      ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
      stopped.get();
    }

    auto shutdown_pending (unsigned int workers) -> void
    {
      with_client
        ( workers
        , [] (auto client, auto state)
          {
            auto pending {std::vector<std::future<int>>{}};
            auto const count
              { std::same_as< typename Policy::admission
                            , mcs::rpc::access_policy::Exclusive
                            >
                  ? std::size_t {1}
                  : std::invoke
                      ( mcs::testing::random::value<std::size_t> {1ul, 25ul}
                      )
              };
            pending.reserve (count);

            std::generate_n
              ( std::back_inserter (pending)
              , count
              , [&]
                {
                  return client.get_future (Handler::Wait{});
                }
              );

            auto started {state->started.get_future()};
            ASSERT_EQ (started.wait_for (timeout), std::future_status::ready);

            auto copy {client.template observe<Observer>()};
            auto stopped {copy.shutdown()};

            auto const value {std::invoke (mcs::testing::random::value<int>{})};
            auto late {client.get_future (Handler::Echo {value})};

            ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
            stopped.get();

            for (auto& response : pending)
            {
              ASSERT_EQ ( response.wait_for (timeout)
                        , std::future_status::ready
                        );
              expect_transport_error ( response
                                     , asio::error::operation_aborted
                                     );
            }

            ASSERT_EQ (late.wait_for (timeout), std::future_status::ready);
            expect_transport_error (late, asio::error::operation_aborted);
          }
        );
    }

    static auto expect_transport_error
      ( std::future<int>& response
      , asio::error_code expected
      ) -> void
    {
      mcs::testing::require_exception
        ( [&]
          {
            response.get();
          }
        , mcs::testing::Assert<mcs::rpc::error::Completion>
            { [expected] (auto const& error)
              {
                mcs::testing::require_exception
                  ( [&]
                    {
                      std::rethrow_exception (error.reason());
                    }
                  , mcs::testing::Assert<std::system_error>
                      { [expected] (auto const& reason)
                        {
                          EXPECT_EQ (reason.code(), expected);
                        }
                      }
                  );
              }
            }
        );
    }
  };

  TYPED_TEST_SUITE (UnifiedClientConnection, Configurations);
}

TYPED_TEST (UnifiedClientConnection, shutdown_pending_one_worker)
{
  this->shutdown_pending (1u);
}

TYPED_TEST (UnifiedClientConnection, shutdown_pending_four_workers)
{
  this->shutdown_pending (4u);
}

TYPED_TEST (UnifiedClientConnection, connection_constructor_is_not_public)
{
  using TestProtocol = typename TypeParam::Protocol;
  using AccessPolicy = typename TypeParam::Policy;
  using Client = mcs::rpc::Client<TestProtocol, AccessPolicy, Handler::Echo>;
  using Connection = mcs::rpc::detail::client::Connection
                   < TestProtocol
                   , typename AccessPolicy::admission
                   >;

  static_assert
    ( !std::is_constructible_v< Client
                             , std::shared_ptr<AccessPolicy>
                             , std::shared_ptr<Connection>
                             >
    );
}

TYPED_TEST ( UnifiedClientConnection
           , copies_and_observed_handles_keep_their_notifications
           )
{
  this->with_client
    ( std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
    , [] (auto client, auto)
      {
        auto completed
          { std::make_shared<std::atomic<unsigned int>> (0u)
          };
        auto nested_completed
          { std::make_shared<std::atomic<unsigned int>> (0u)
          };

        auto observed {client.template observe<CountingObserver> (completed)};
        auto copy {observed};
        auto nested
          { observed.template observe<CountingObserver> (nested_completed)
          };

        {
          auto const value {std::invoke (mcs::testing::random::value<int>{})};
          EXPECT_EQ (client (Handler::Echo {value}), value);
          EXPECT_EQ (completed->load(), 0u);
        }

        {
          auto const value {std::invoke (mcs::testing::random::value<int>{})};
          EXPECT_EQ (copy (Handler::Echo {value}), value);
          EXPECT_EQ (completed->load(), 1u);
        }

        {
          auto const value {std::invoke (mcs::testing::random::value<int>{})};
          EXPECT_EQ (observed (Handler::Echo {value}), value);
          EXPECT_EQ (completed->load(), 2u);
        }

        {
          auto const value {std::invoke (mcs::testing::random::value<int>{})};
          EXPECT_EQ (nested (Handler::Echo {value}), value);
          EXPECT_EQ (completed->load(), 3u);
          EXPECT_EQ (nested_completed->load(), 1u);
        }
      }
    );
}

TYPED_TEST (UnifiedClientConnection, receive_operation_owns_socket)
{
  using TestProtocol = typename TypeParam::Protocol;

  auto context {asio::io_context{}};
  auto socket {std::make_shared<typename TestProtocol::socket> (context)};
  auto const weak {std::weak_ptr<typename TestProtocol::socket> {socket}};

  {
    auto operation
      { mcs::rpc::detail::receive_buffer_with_header<std::size_t> (socket)
      };

    socket.reset();

    EXPECT_TRUE (operation.valid());
    EXPECT_FALSE (weak.expired());
  }

  EXPECT_TRUE (weak.expired());
}

TYPED_TEST (UnifiedClientConnection, dispatch_operation_owns_context)
{
  using TestProtocol = typename TypeParam::Protocol;
  using Dispatcher = mcs::rpc::Dispatcher<Handler, Handler::Wait>;

  auto context {asio::io_context{}};
  auto state {std::make_shared<State> (context.get_executor())};
  auto started {state->started.get_future()};
  auto dispatcher {std::make_shared<Dispatcher> (state)};
  auto socket {std::make_shared<typename TestProtocol::socket> (context)};
  auto const weak_dispatcher {std::weak_ptr<Dispatcher> {dispatcher}};
  auto const weak_socket
    { std::weak_ptr<typename TestProtocol::socket> {socket}
    };

  auto operation
    { Dispatcher::template dispatch<TestProtocol>
      ( mcs::rpc::detail::ReceivedFrame<typename Dispatcher::Header>
        { .header = typename Dispatcher::Header
            { .call_id = mcs::rpc::detail::CallID{}
            , .index = mcs::rpc::detail::CommandIndex{}
            }
        , .buffer = mcs::rpc::detail::Buffer {0}
        }
      , typename Dispatcher::template RequestContext<TestProtocol>
        { .dispatcher = std::move (dispatcher)
        , .socket = std::move (socket)
        }
      )
    };

  EXPECT_FALSE (weak_dispatcher.expired());
  EXPECT_FALSE (weak_socket.expired());

  auto result
    { asio::co_spawn
      ( context
      , std::move (operation)
      , asio::use_future
      )
    };
  while (context.poll() > 0)
  {
  }

  ASSERT_EQ
    ( started.wait_for (std::chrono::seconds {0})
    , std::future_status::ready
    );
  EXPECT_FALSE (weak_dispatcher.expired());
  EXPECT_FALSE (weak_socket.expired());

  std::ignore = state->gate.cancel();
  context.restart();
  std::ignore = context.run();

  ASSERT_EQ
    ( result.wait_for (std::chrono::seconds {0})
    , std::future_status::ready
    );
  std::ignore = result.get();

  EXPECT_TRUE (weak_dispatcher.expired());
  EXPECT_TRUE (weak_socket.expired());
}

TYPED_TEST (UnifiedClientConnection, send_factory_failure_is_delivered)
{
  using TestProtocol = typename TypeParam::Protocol;
  using AccessPolicy = typename TypeParam::Policy;
  using Connection = mcs::rpc::detail::client::Connection
                   < TestProtocol
                   , typename AccessPolicy::admission
                   >;

  auto context
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
        { std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
        }
      }
    };
  auto connection
    { std::make_shared<Connection>
      ( std::make_shared<typename TestProtocol::socket>
        ( context.get_executor()
        )
      )
    };
  auto promise {std::promise<int>{}};
  auto response {promise.get_future()};

  connection->submit
    ( [] (auto, auto) -> asio::awaitable<void>
      {
        throw std::runtime_error {"send factory failure"};
      }
    , mcs::rpc::detail::Completion
      { std::move (promise)
      , std::make_shared<AccessPolicy>()
      }
    );

  ASSERT_EQ (response.wait_for (timeout), std::future_status::ready);

  mcs::testing::require_exception
    ( [&]
      {
        response.get();
      }
    , mcs::testing::Assert<mcs::rpc::error::Completion>
        { [] (auto const& error)
          {
            ASSERT_STREQ
              ( error.what()
              , "rpc::error::Completion: send factory failure"
              );

            mcs::testing::require_exception
              ( [&]
                {
                  std::rethrow_exception (error.reason());
                }
              , mcs::testing::assert_type_and_what<std::runtime_error>
                  ( "send factory failure"
                  )
              );
          }
        }
    );

  auto stopped {connection->shutdown()};
  ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
  stopped.get();
}

TYPED_TEST (UnifiedClientConnection, serialization_failure_is_deferred)
{
  for (auto api {0}; api != 3; ++api)
  {
    this->with_client
      ( 1u
      , [api] (auto client, auto)
        {
          auto const value
            { std::invoke (mcs::testing::random::value<int> {0, 1000})
            };
          auto command {Handler::ThrowOnSave {value}};
          auto response {std::future<int>{}};

          if (api == 0)
          {
            ASSERT_NO_THROW
              (response = client.get_future (std::move (command)));
          }
          else if (api == 1)
          {
            ASSERT_NO_THROW
              (response = client.get_future (std::cref (command)));
          }
          else
          {
            ASSERT_NO_THROW
              ( response = client.template async_call<Handler::ThrowOnSave>
                  (value)
              );
          }

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
                      , mcs::testing::assert_type_or_derived_type
                          < mcs::serialization::error::Save
                          >()
                      , mcs::testing::assert_type_and_what<std::runtime_error>
                          ( "serialization failure"
                          )
                      );
                  }
                }
            );
        }
      );
  }
}

TYPED_TEST ( UnifiedClientConnection
           , command_constructor_failure_is_immediate
           )
{
  this->with_client
    ( 1u
    , [] (auto client, auto)
      {
        mcs::testing::require_exception
          ( [&]
            {
              std::ignore
                = client.template async_call<Handler::ThrowOnSave> (-1)
                ;
            }
          , mcs::testing::assert_type_and_what<std::runtime_error>
              ( "construction failure"
              )
          );

        auto const value {std::invoke (mcs::testing::random::value<int>{})};
        EXPECT_EQ (client (Handler::Echo {value}), value);
      }
    );
}

TYPED_TEST (UnifiedClientConnection, disconnect_retains_terminal_reason)
{
  this->with_client
    ( std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
    , [] (auto client, auto)
      {
        auto const value {std::invoke (mcs::testing::random::value<int>{})};
        auto broken {client.get_future (Handler::Broken {value})};
        ASSERT_EQ (broken.wait_for (timeout), std::future_status::ready);
        TestFixture::expect_transport_error (broken, asio::error::eof);

        auto const late_value
          { std::invoke (mcs::testing::random::value<int>{})
          };
        auto late {client.get_future (Handler::Echo {late_value})};
        ASSERT_EQ (late.wait_for (timeout), std::future_status::ready);
        TestFixture::expect_transport_error (late, asio::error::eof);
      }
    );
}

TYPED_TEST ( UnifiedClientConnection
           , admission_matches_policy_without_blocking_submission
           )
{
  using AccessPolicy = typename TypeParam::Policy;

  this->with_client
    ( std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
    , [] (auto client, auto state)
      {
        auto waiting {client.get_future (Handler::Wait{})};

        auto started {state->started.get_future()};
        ASSERT_EQ (started.wait_for (timeout), std::future_status::ready);

        auto const value
          { std::invoke (mcs::testing::random::value<int>{})
          };
        auto echo {client.get_future (Handler::Echo {value})};

        if constexpr ( std::same_as< typename AccessPolicy::admission
                                   , mcs::rpc::access_policy::Sequential
                                   >
                     )
        {
          EXPECT_EQ
            ( echo.wait_for (std::chrono::milliseconds {50})
            , std::future_status::timeout
            );
        }
        else
        {
          ASSERT_EQ (echo.wait_for (timeout), std::future_status::ready);

          if constexpr ( std::same_as< typename AccessPolicy::admission
                                     , mcs::rpc::access_policy::Exclusive
                                     >
                       )
          {
            mcs::testing::require_exception
              ( [&]
                {
                  echo.get();
                }
              , mcs::testing::Assert<mcs::rpc::error::Completion>
                  { [] (auto const& error)
                    {
                      mcs::testing::require_exception
                        ( [&]
                          {
                            std::rethrow_exception (error.reason());
                          }
                        , mcs::testing::assert_type_and_what<std::logic_error>
                            ( "Overlapping calls on an Exclusive client"
                            )
                        );
                    }
                  }
              );
          }
          else
          {
            EXPECT_EQ (echo.get(), value);
          }
        }

        asio::post
          ( state->strand
          , [state]
            {
              state->gate.expires_at (std::chrono::steady_clock::now());
            }
          );

        ASSERT_EQ (waiting.wait_for (timeout), std::future_status::ready);
        EXPECT_EQ (waiting.get(), 42);

        if constexpr ( std::same_as< typename AccessPolicy::admission
                                  , mcs::rpc::access_policy::Sequential
                                  >
                     )
        {
          ASSERT_EQ (echo.wait_for (timeout), std::future_status::ready);
          EXPECT_EQ (echo.get(), value);
        }
      }
    );
}

TYPED_TEST (UnifiedClientConnection, idle_shutdown)
{
  this->with_client
    ( std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
    , [] (auto, auto) {}
    );
}

TYPED_TEST ( UnifiedClientConnection
           , blocked_write_is_drained_before_failure_delivery
           )
{
  this->with_client
    ( std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
    , [] (auto client, auto)
      {
        auto destroyed {std::make_shared<std::atomic<bool>> (false)};
        auto saving {std::make_shared<std::promise<void>>()};
        auto started {saving->get_future()};
        auto observed {client.template observe<LifetimeObserver> (destroyed)};

        auto const mib
          { std::invoke (mcs::testing::random::value<std::size_t> {8ul, 16ul})
          };
        auto response
          { observed.get_future
            (Handler::Large {mib << 20ul, saving, destroyed})
          };

        ASSERT_EQ (started.wait_for (timeout), std::future_status::ready);

        auto stopped {observed.shutdown()};
        ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
        stopped.get();

        ASSERT_EQ (response.wait_for (timeout), std::future_status::ready);

        TestFixture::expect_transport_error
          ( response
          , asio::error::operation_aborted
          );

        EXPECT_TRUE (destroyed->load());
      }
    , true
    );
}

TYPED_TEST (UnifiedClientConnection, streaming_failure_is_deferred)
{
  this->with_client
    ( 1u
    , [] (auto client, auto)
      {
        auto response {client.get_future (Handler::ThrowOnStream{})};

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
                    , mcs::testing::assert_type_and_what<std::runtime_error>
                        ( "stream failure"
                        )
                    );
                }
              }
          );
      }
    );
}

TYPED_TEST ( UnifiedClientConnection
           , rejected_call_releases_command_before_failure_delivery
           )
{
  this->with_client
    ( std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
    , [] (auto client, auto)
      {
        auto stopped {client.shutdown()};
        ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
        stopped.get();

        auto destroyed {std::make_shared<std::atomic<bool>> (false)};
        auto saving {std::make_shared<std::promise<void>>()};

        auto observed {client.template observe<LifetimeObserver> (destroyed)};

        auto const mib
          { std::invoke (mcs::testing::random::value<std::size_t> {8ul, 16ul})
          };

        auto response
          { observed.get_future
            (Handler::Large {mib << 20ul, saving, destroyed})
          };

        ASSERT_EQ (response.wait_for (timeout), std::future_status::ready);

        TestFixture::expect_transport_error
          ( response
          , asio::error::operation_aborted
          );

        EXPECT_TRUE (destroyed->load());
      }
    );
}

TYPED_TEST ( UnifiedClientConnection
           , handler_exception_does_not_stall_admission
           )
{
  using AccessPolicy = typename TypeParam::Policy;

  this->with_client
    ( std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
    , [] (auto client, auto)
      {
        auto failed {client.get_future (Handler::Throw{})};

        if constexpr ( std::same_as< typename AccessPolicy::admission
                                  , mcs::rpc::access_policy::Exclusive
                                  >
                     )
        {
          ASSERT_EQ (failed.wait_for (timeout), std::future_status::ready);
        }

        auto const value {std::invoke (mcs::testing::random::value<int>{})};
        auto response {client.get_future (Handler::Echo {value})};

        ASSERT_EQ (failed.wait_for (timeout), std::future_status::ready);

        mcs::testing::require_exception
          ( [&]
            {
              failed.get();
            }
          , mcs::testing::assert_type_and_what<mcs::rpc::error::HandlerError>
              ( "rpc::error::HandlerError: handler failure"
              )
          );

        ASSERT_EQ (response.wait_for (timeout), std::future_status::ready);
        EXPECT_EQ (response.get(), value);
      }
    );
}

TYPED_TEST (UnifiedClientConnection, shutdown_races_submission)
{
  using AccessPolicy = typename TypeParam::Policy;

  this->with_client
    ( std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
    , [] (auto client, auto)
      {
        auto submitting {std::promise<void>{}};
        auto started {submitting.get_future()};

        auto const count
          { std::same_as< typename AccessPolicy::admission
                        , mcs::rpc::access_policy::Exclusive
                        >
              ? std::size_t {1}
              : std::invoke
                  (mcs::testing::random::value<std::size_t> {10ul, 100ul})
          };

        auto submissions
          { std::async
              ( std::launch::async
              , [client, &submitting, count {count}]
                {
                  auto pending {std::vector<std::future<int>>{}};
                  pending.reserve (count);

                  submitting.set_value();

                  std::generate_n
                    ( std::back_inserter (pending)
                    , count
                    , [&]
                      {
                        return client.get_future (Handler::Wait{});
                      }
                    );

                  return pending;
                }
              )
          };

        ASSERT_EQ (started.wait_for (timeout), std::future_status::ready);

        auto stopped {client.shutdown()};

        ASSERT_EQ (submissions.wait_for (timeout), std::future_status::ready);

        auto pending {submissions.get()};

        ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
        stopped.get();

        for (auto& response : pending)
        {
          ASSERT_EQ (response.wait_for (timeout), std::future_status::ready);

          TestFixture::expect_transport_error
            ( response
            , asio::error::operation_aborted
            );
        }
      }
    );
}

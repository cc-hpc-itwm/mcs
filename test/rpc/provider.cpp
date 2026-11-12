// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/error.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <asio/post.hpp>
#include <chrono>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
  constexpr auto timeout {std::chrono::seconds {5}};

  struct Handler
  {
    struct Echo
    {
      using Response = int;

      int value;
    };

    explicit Handler (std::shared_ptr<int> lifetime)
      : _lifetime {std::move (lifetime)}
    {}

    [[nodiscard]] auto operator() (Echo command) const noexcept -> int
    {
      return command.value;
    }

  private:
    std::shared_ptr<int> _lifetime;
  };

  using Dispatcher = mcs::rpc::Dispatcher<Handler, Handler::Echo>;
  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;

  template<typename Protocol>
    struct Provider : public mcs::testing::random::Test
  {
    using Listener
      = mcs::rpc::Provider<Protocol, Dispatcher, std::shared_ptr<int>>
      ;

    static_assert (!std::is_copy_constructible_v<Listener>);
    static_assert (!std::is_move_constructible_v<Listener>);
  };

  TYPED_TEST_SUITE (Provider, Protocols);

  TYPED_TEST (Provider, destruction_before_context_runs_releases_arguments)
  {
    auto context {asio::io_context{}};
    auto lifetime {std::make_shared<int> (42)};
    auto const weak {std::weak_ptr<int> {lifetime}};

    {
      auto provider
        { mcs::rpc::make_provider<TypeParam, Dispatcher>
            ({}, context, std::move (lifetime))
        };
    }

    EXPECT_FALSE (weak.expired());
    context.run();
    EXPECT_TRUE (weak.expired());
  }

  TYPED_TEST (Provider, stop_before_context_runs_is_idempotent)
  {
    auto context {asio::io_context{}};
    auto const provider
      { mcs::rpc::make_provider<TypeParam, Dispatcher>
        ( {}
        , context
        , std::make_shared<int>
          ( std::invoke (mcs::testing::random::value<int>{})
          )
        )
      };
    auto first {provider.stop_accepting()};
    auto second {provider.stop_accepting()};

    EXPECT_EQ ( first.wait_for (std::chrono::seconds {0})
              , std::future_status::timeout
              );
    context.run();

    ASSERT_EQ (first.wait_for (timeout), std::future_status::ready);
    ASSERT_EQ (second.wait_for (timeout), std::future_status::ready);
    EXPECT_NO_THROW (first.get());
    EXPECT_NO_THROW (second.get());
  }

  TYPED_TEST (Provider, destruction_cancels_pending_accept)
  {
    auto context {asio::io_context{}};
    auto lifetime
      { std::make_shared<int>
        ( std::invoke (mcs::testing::random::value<int>{})
        )
      };
    auto const weak {std::weak_ptr<int> {lifetime}};

    {
      auto provider
        { mcs::rpc::make_provider<TypeParam, Dispatcher>
            ({}, context, std::move (lifetime))
        };

      context.poll();
      EXPECT_FALSE (weak.expired());
    }

    context.run();
    EXPECT_TRUE (weak.expired());
  }

  TYPED_TEST ( Provider
             , stopped_context_can_be_destroyed_without_running_cleanup
             )
  {
    auto lifetime
      { std::make_shared<int>
        ( std::invoke (mcs::testing::random::value<int>{})
        )
      };
    auto const weak {std::weak_ptr<int> {lifetime}};

    {
      auto context {asio::io_context{}};
      auto provider
        { mcs::rpc::make_provider<TypeParam, Dispatcher>
            ({}, context, std::move (lifetime))
        };

      context.poll();
      context.stop();
    }

    EXPECT_TRUE (weak.expired());
  }

  template<typename Protocol>
    auto check_established_connection (unsigned int workers) -> void
  {
    auto const value
      { std::invoke (mcs::testing::random::value<int> {-1000, 1000})
      };
    auto context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {workers}
        }
      };

    auto const provider
      { mcs::rpc::make_provider<Protocol, Dispatcher>
        ( {}
        , context
        , std::make_shared<int>
          ( std::invoke (mcs::testing::random::value<int>{})
          )
        )
      };
    auto const client
      { mcs::rpc::make_client< Protocol
                            , Dispatcher
                            , mcs::rpc::access_policy::Exclusive
                            >
          (context, provider.local_endpoint())
      };

    auto stopped {provider.stop_accepting()};

    ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
    EXPECT_NO_THROW (stopped.get());

    auto result {client.get_future (Handler::Echo {value})};

    ASSERT_EQ (result.wait_for (timeout), std::future_status::ready);
    EXPECT_EQ (result.get(), value);

    auto again {provider.stop_accepting()};

    ASSERT_EQ (again.wait_for (timeout), std::future_status::ready);
    EXPECT_NO_THROW (again.get());

    auto peer {typename Protocol::socket {context}};
    auto error {asio::error_code{}};

    peer.connect (provider.local_endpoint(), error);
    EXPECT_EQ (error, asio::error::connection_refused);
  }

  TYPED_TEST (Provider, stopping_keeps_established_connections_one_worker)
  {
    check_established_connection<TypeParam> (1u);
  }

  TYPED_TEST (Provider, stopping_keeps_established_connections_four_workers)
  {
    check_established_connection<TypeParam> (4u);
  }

  TYPED_TEST (Provider, stop_on_executor_worker_does_not_block)
  {
    auto requested {std::promise<std::shared_future<void>>{}};
    auto ready {requested.get_future()};

    auto context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        }
      };

    auto provider
      { std::make_unique<typename TestFixture::Listener>
          ( typename TypeParam::endpoint{}
          , context
          , std::make_shared<int>
            ( std::invoke (mcs::testing::random::value<int>{})
            )
          )
      };

    asio::post
      ( context
      , [provider = std::move (provider), &requested]
        {
          requested.set_value (provider->stop_accepting());
        }
      );

    ASSERT_EQ (ready.wait_for (timeout), std::future_status::ready);

    auto stopped {ready.get()};
    ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
    EXPECT_NO_THROW (stopped.get());
  }

  template<typename Protocol>
    auto check_destruction_races_connections (unsigned int workers) -> void
  {
    using Listener
      = mcs::rpc::Provider<Protocol, Dispatcher, std::shared_ptr<int>>;

    auto const N
      { std::invoke (mcs::testing::random::value<unsigned> {32u, 256u})
      };
    auto lifetimes {std::vector<std::weak_ptr<int>>{}};
    lifetimes.reserve (N);

    {
      auto context
        { mcs::rpc::ScopedRunningIOContext
          { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {workers}
          }
        };

      auto random_value {mcs::testing::random::value<int>{}};

      for (auto iteration {0u}; iteration < N; ++iteration)
      {
        auto lifetime {std::make_shared<int> (std::invoke (random_value))};

        lifetimes.emplace_back (lifetime);

        auto provider
          { std::make_unique<Listener>
              ( typename Protocol::endpoint{}
              , context
              , std::move (lifetime)
              )
          };

        auto socket {std::make_shared<typename Protocol::socket> (context)};
        auto connected {std::make_shared<std::promise<asio::error_code>>()};
        auto ready {connected->get_future()};

        socket->async_connect
          ( provider->local_endpoint()
          , [socket, connected] (asio::error_code error)
            {
              auto close_error {asio::error_code{}};

              socket->close (close_error);

              EXPECT_FALSE (close_error);
              connected->set_value (error);
            }
          );

        provider.reset();

        ASSERT_EQ (ready.wait_for (timeout), std::future_status::ready);

        auto const error {ready.get()};

        EXPECT_TRUE ( !error
                    || error == asio::error::connection_refused
                    || error == asio::error::connection_aborted
                    || error == asio::error::connection_reset
                    ) << error.message()
          ;
      }
    }

    EXPECT_TRUE
      ( std::ranges::all_of
        ( lifetimes
        , [] (auto const& lifetime) noexcept
          {
            return lifetime.expired();
          }
        )
      );
  }

  TYPED_TEST (Provider, destruction_races_connections_one_worker)
  {
    check_destruction_races_connections<TypeParam> (1u);
  }

  TYPED_TEST (Provider, destruction_races_connections_four_workers)
  {
    check_destruction_races_connections<TypeParam> (4u);
  }
}

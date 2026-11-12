// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <array>
#include <asio/any_io_executor.hpp>
#include <asio/error.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <cerrno>
#include <chrono>
#include <concepts>
#include <exception>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/detail/provider/State.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/overloaded.hpp>
#include <memory>
#include <string>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>

namespace
{
  namespace policy = mcs::rpc::detail::provider::detail::retry::policy;

  constexpr auto timeout {std::chrono::seconds {5}};

  struct Diagnostics
  {
    Diagnostics()
    {
      ::testing::internal::CaptureStderr();
    }

    ~Diagnostics()
    {
      if (_capturing)
      {
        std::ignore = ::testing::internal::GetCapturedStderr();
      }
    }

    Diagnostics (Diagnostics const&) = delete;
    Diagnostics (Diagnostics&&) = delete;
    auto operator= (Diagnostics const&) -> Diagnostics& = delete;
    auto operator= (Diagnostics&&) -> Diagnostics& = delete;

    [[nodiscard]] auto STDerr() -> std::string
    {
      _capturing = false;

      return ::testing::internal::GetCapturedStderr();
    }

  private:
    bool _capturing {true};
  };

  struct HandlerContext
  {
    HandlerContext
      ( asio::any_io_executor expected_executor
      , std::shared_ptr<bool> failure
      )
        : executor {std::move (expected_executor)}
        , fail {std::move (failure)}
    {}

    HandlerContext (HandlerContext const& other)
      : executor {other.executor}
      , fail {other.fail}
    {
      if (*fail)
      {
        throw std::system_error
          { asio::error_code {asio::error::no_descriptors}
          };
      }
    }

    HandlerContext (HandlerContext&&) = default;
    ~HandlerContext() = default;
    auto operator= (HandlerContext const&) -> HandlerContext& = delete;
    auto operator= (HandlerContext&&) -> HandlerContext& = delete;

    asio::any_io_executor executor;
    std::shared_ptr<bool> fail;
  };

  struct Handler
  {
    struct Echo
    {
      using Response = int;

      int value;
    };

    explicit Handler (HandlerContext context)
      : _context {std::move (context)}
    {}

    template<typename Socket>
      [[nodiscard]] auto operator()
        ( Echo command
        , Socket& socket
        ) const noexcept -> int
    {
      EXPECT_EQ (socket.get_executor(), _context.executor);

      return command.value;
    }

  private:
    HandlerContext _context;
  };

  using Dispatcher = mcs::rpc::Dispatcher<Handler, Handler::Echo>;

  template<typename Protocol>
    struct ProviderAcceptLoop : public mcs::testing::random::Test
  {
    using State = mcs::rpc::detail::provider::State<Protocol>;
    using Listener = mcs::rpc::Provider<Protocol, Dispatcher, HandlerContext>;

    template<typename Executor>
      [[nodiscard]] static auto make_provider
        ( Executor& executor
        , std::shared_ptr<bool> fail = std::make_shared<bool> (false)
        ) -> std::unique_ptr<Listener>
    {
      return std::make_unique<Listener>
        ( typename Protocol::endpoint{}
        , executor
        , HandlerContext {executor.get_executor(), std::move (fail)}
        );
    }

    template<typename Expected>
      static auto require_policy (asio::error_code error) -> void
    {
      SCOPED_TRACE (error.message());

      std::visit
        ( mcs::util::overloaded
          { [] (Expected) noexcept
            {
            }
          , [] (auto const&)
            {
              ADD_FAILURE() << "Unexpected accept retry policy";
            }
          }
        , mcs::rpc::detail::provider::detail::retry_policy<Protocol> (error)
        );
    }

    static auto require_error
      ( std::shared_future<void> const& stopped
      , asio::error_code expected
      , std::string const& message
      ) -> void
    {
      mcs::testing::require_exception
        ( [&stopped]
          {
            stopped.get();
          }
        , mcs::testing::Assert<std::system_error>
            { [&] (auto const& error)
              {
                EXPECT_EQ (typeid (error), typeid (std::system_error));
                EXPECT_EQ (error.code(), expected);
                EXPECT_EQ (error.what(), message);
              }
            }
        );
    }
  };

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;

  TYPED_TEST_SUITE (ProviderAcceptLoop, Protocols);
}

namespace
{
  TYPED_TEST (ProviderAcceptLoop, dispatcher_constraints_match_invocation)
  {
    namespace provider = mcs::rpc::detail::provider;

    using ListenerState = typename TestFixture::State;
    using Socket = typename TypeParam::socket;
    using OtherProtocol = std::conditional_t
      < std::is_same_v<TypeParam, asio::ip::tcp>
      , asio::local::stream_protocol
      , asio::ip::tcp
      >;
    using ByValue = void (*) (Socket);
    using ByRvalue = void (*) (Socket&&) noexcept;
    using ByConstReference = int (*) (Socket const&);
    using ByReference = void (*) (Socket&);
    using WrongProtocol = void (*) (typename OtherProtocol::socket);

    static_assert
      ( std::constructible_from
          < ListenerState
          , asio::io_context&
          , typename TypeParam::endpoint
          >
      );
    static_assert
      ( !requires (ListenerState const& state)
        {
          state.executor();
        }
      );
    static_assert
      ( !requires (ListenerState const& state)
        {
          state.local_endpoint();
        }
      );
    static_assert
      ( !requires (ListenerState& state)
        {
          state.stop_accepting();
        }
      );
    static_assert
      ( !requires (ListenerState& state)
        {
          state.request_stop();
        }
      );
    static_assert
      ( !requires (ListenerState& state)
        {
          state.close();
        }
      );
    static_assert
      ( !requires (ListenerState& state)
        {
          state.finish (std::exception_ptr{});
        }
      );

    struct LvalueOnly
    {
      LvalueOnly() = default;
      LvalueOnly (LvalueOnly&&) = default;
      ~LvalueOnly() = default;
      LvalueOnly (LvalueOnly const&) = delete;
      auto operator= (LvalueOnly const&) -> LvalueOnly& = delete;
      auto operator= (LvalueOnly&&) -> LvalueOnly& = delete;
      auto operator() (Socket) & -> void;
    };

    struct RvalueOnly
    {
      auto operator() (Socket) && -> void;
    };

    struct Immovable
    {
      Immovable() = default;
      Immovable (Immovable&&) = delete;
      ~Immovable() = default;
      Immovable (Immovable const&) = delete;
      auto operator= (Immovable const&) -> Immovable& = delete;
      auto operator= (Immovable&&) -> Immovable& = delete;
      auto operator() (Socket) -> void;
    };

    constexpr auto accepts
      { []<typename Dispatch>() noexcept
        {
          constexpr auto expected
            { provider::detail::is_dispatch<Dispatch, TypeParam>
            };

          static_assert
            ( !requires (ListenerState& state, Dispatch dispatch)
              {
                state.start (std::move (dispatch));
              }
            );
          static_assert
            ( expected == requires
                ( std::shared_ptr<ListenerState> state
                , Dispatch dispatch
                , provider::detail::AcceptClients<TypeParam> const& accept
                )
              {
                std::invoke
                  ( accept
                  , std::move (state)
                  , std::move (dispatch)
                  );
              }
            );

          return expected;
        }
      };

    static_assert (accepts.template operator()<ByValue>());
    static_assert (accepts.template operator()<ByRvalue>());
    static_assert (accepts.template operator()<ByConstReference>());
    static_assert (accepts.template operator()<LvalueOnly>());
    static_assert (!accepts.template operator()<ByReference>());
    static_assert (!accepts.template operator()<WrongProtocol>());
    static_assert (!accepts.template operator()<RvalueOnly>());
    static_assert (!accepts.template operator()<Immovable>());
    static_assert (!accepts.template operator()<int>());
    static_assert (!provider::detail::is_dispatch<ByValue, int>);
    static_assert (!std::copy_constructible<LvalueOnly>);
    static_assert (!std::movable<LvalueOnly>);
  }

  TYPED_TEST (ProviderAcceptLoop, stopping_pending_accept_is_quiet)
  {
    auto diagnostics {Diagnostics{}};
    auto context {asio::io_context{}};
    auto lifetime {std::make_shared<bool> (false)};
    auto const weak {std::weak_ptr<bool> {lifetime}};
    auto provider {this->make_provider (context, std::move (lifetime))};

    context.poll();

    auto stopped {provider->stop_accepting()};

    EXPECT_FALSE (weak.expired());
    provider.reset();
    context.run();

    ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
    EXPECT_NO_THROW (stopped.get());
    EXPECT_TRUE (weak.expired());
    EXPECT_TRUE (diagnostics.STDerr().empty());
  }

  TYPED_TEST (ProviderAcceptLoop, transient_errors_select_prompt_retry)
  {
    auto const errors
      { std::array<asio::error_code, 8>
        { asio::error::connection_aborted
        , asio::error::interrupted
        , asio::error::try_again
        , asio::error::would_block
        , std::make_error_code (std::errc::connection_aborted)
        , std::make_error_code (std::errc::interrupted)
        , std::make_error_code (std::errc::resource_unavailable_try_again)
        , std::make_error_code (std::errc::operation_would_block)
        }
      };

    for (auto const error : errors)
    {
      this->template require_policy<policy::Prompt> (error);
    }
  }

  TYPED_TEST (ProviderAcceptLoop, resource_errors_select_backoff)
  {
    static_assert
      ( policy::Backoff::_initial_delay == std::chrono::microseconds {250}
      );
    static_assert
      ( policy::Backoff::_maximum_delay == std::chrono::microseconds {2500}
      );

    auto const errors
      { std::array<asio::error_code, 8>
        { asio::error::no_descriptors
        , asio::error::no_buffer_space
        , asio::error::no_memory
        , asio::error_code {ENFILE, std::system_category()}
        , std::make_error_code (std::errc::too_many_files_open)
        , std::make_error_code (std::errc::no_buffer_space)
        , std::make_error_code (std::errc::not_enough_memory)
        , std::make_error_code (std::errc::too_many_files_open_in_system)
        }
      };

    for (auto const error : errors)
    {
      this->template require_policy<policy::Backoff> (error);
    }
  }

  TYPED_TEST
    ( ProviderAcceptLoop
    , pending_network_errors_are_protocol_specific
    )
  {
    using Expected = std::conditional_t
      < std::is_same_v<TypeParam, asio::ip::tcp>
      , policy::Prompt
      , policy::None
      >;

    auto const errors
      { std::array<int, 8>
        { ENETDOWN
        , EPROTO
        , ENOPROTOOPT
        , EHOSTDOWN
        , ENONET
        , EHOSTUNREACH
        , EOPNOTSUPP
        , ENETUNREACH
        }
      };

    auto const categories
      { std::array<std::error_category const*, 2>
        { std::addressof (std::system_category())
        , std::addressof (std::generic_category())
        }
      };

    for (auto const* category : categories)
    {
      for (auto const value : errors)
      {
        this->template require_policy<Expected>
          ( asio::error_code {value, *category}
          );
      }
    }
  }

  TYPED_TEST (ProviderAcceptLoop, fatal_errors_do_not_select_retry)
  {
    auto const errors
      { std::array<asio::error_code, 5>
        { asio::error::bad_descriptor
        , asio::error::invalid_argument
        , asio::error::operation_aborted
        , asio::error::not_socket
        , asio::error_code {EMFILE, asio::error::get_misc_category()}
        }
      };

    for (auto const error : errors)
    {
      this->template require_policy<policy::None> (error);
    }
  }

  TYPED_TEST (ProviderAcceptLoop, successful_accepts_dispatch_connections)
  {
    auto const value {std::invoke (mcs::testing::random::value<int>{})};
    auto diagnostics {Diagnostics{}};

    auto context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        }
      };
    auto provider {this->make_provider (context)};

    auto const first
      { mcs::rpc::make_client< TypeParam
                             , Dispatcher
                             , mcs::rpc::access_policy::Exclusive
                             >
          (context, provider->local_endpoint())
      };
    auto const second
      { mcs::rpc::make_client< TypeParam
                             , Dispatcher
                             , mcs::rpc::access_policy::Exclusive
                             >
          (context, provider->local_endpoint())
      };
    auto first_result {first.get_future (Handler::Echo {value})};
    auto second_result {second.get_future (Handler::Echo {value})};

    ASSERT_EQ (first_result.wait_for (timeout), std::future_status::ready);
    ASSERT_EQ (second_result.wait_for (timeout), std::future_status::ready);
    EXPECT_EQ (first_result.get(), value);
    EXPECT_EQ (second_result.get(), value);

    auto stopped {provider->stop_accepting()};

    ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
    EXPECT_NO_THROW (stopped.get());
    EXPECT_TRUE (diagnostics.STDerr().empty());
  }

  TYPED_TEST (ProviderAcceptLoop, dispatch_errors_are_not_retried_as_accepts)
  {
    auto diagnostics {Diagnostics{}};
    auto context {asio::io_context{}};
    auto fail {std::make_shared<bool> (false)};

    auto provider {this->make_provider (context, fail)};

    auto peer {typename TypeParam::socket {context}};
    auto const error {asio::error_code {asio::error::no_descriptors}};

    // Fail while copying handler arguments into connection dispatch.
    *fail = true;
    peer.connect (provider->local_endpoint());
    context.run();

    auto stopped {provider->stop_accepting()};
    auto const output {diagnostics.STDerr()};
    auto const expected {std::system_error {error}};

    ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
    this->require_error (stopped, error, expected.what());

    EXPECT_NE (output.find (expected.what()), std::string::npos);
    EXPECT_EQ (output.find ("retrying"), std::string::npos);
  }

  template<typename Protocol, typename Fixture>
    auto check_rpc_accepts_connections (unsigned int workers) -> void
  {
    auto const value {std::invoke (mcs::testing::random::value<int>{})};
    auto diagnostics {Diagnostics{}};

    auto context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {workers}
        }
      };
    auto provider {Fixture::make_provider (context)};

    auto const client
      { mcs::rpc::make_client< Protocol
                             , Dispatcher
                             , mcs::rpc::access_policy::Exclusive
                             >
          (context, provider->local_endpoint())
      };
    auto result {client.get_future (Handler::Echo {value})};

    ASSERT_EQ (result.wait_for (timeout), std::future_status::ready);
    EXPECT_EQ (result.get(), value);

    auto stopped {provider->stop_accepting()};

    ASSERT_EQ (stopped.wait_for (timeout), std::future_status::ready);
    EXPECT_NO_THROW (stopped.get());
    EXPECT_TRUE (diagnostics.STDerr().empty());
  }

  TYPED_TEST (ProviderAcceptLoop, rpc_accepts_connections_one_worker)
  {
    check_rpc_accepts_connections<TypeParam, TestFixture> (1u);
  }

  TYPED_TEST (ProviderAcceptLoop, rpc_accepts_connections_four_workers)
  {
    check_rpc_accepts_connections<TypeParam, TestFixture> (4u);
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/any_io_executor.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/error.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <asio/post.hpp>
#include <asio/redirect_error.hpp>
#include <asio/steady_timer.hpp>
#include <asio/strand.hpp>
#include <asio/use_awaitable.hpp>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/error/Completion.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/error/Load.hpp>
#include <mcs/serialization/error/NotEnoughBytes.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
  struct RequestsAwaitingMalformedRequest
    : std::enable_shared_from_this<RequestsAwaitingMalformedRequest>
  {
    explicit RequestsAwaitingMalformedRequest
      ( asio::any_io_executor executor
      , std::size_t number_of_requests
      , unsigned int expected_value
      , std::chrono::seconds timeout
      )
        : _executor {asio::make_strand (std::move (executor))}
        , _remaining_requests {number_of_requests}
        , _request_results (number_of_requests)
        , _expected_value {expected_value}
        , _timeout {timeout}
    {}

    [[nodiscard]] auto wait_for_malformed_request() -> asio::awaitable<void>
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
      ( std::shared_ptr<RequestsAwaitingMalformedRequest> requests
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
          throw std::system_error {error, "Waiting for malformed-request gate"};
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
    struct BrokenSerialization
    {
      using Response = void;

      explicit BrokenSerialization (int value) noexcept
          : _value {value}
      {}

      explicit BrokenSerialization (mcs::serialization::IArchive& archive)
          : _value {mcs::serialization::load<int> (archive)}
      {}

      auto save
        ( mcs::serialization::OArchive& archive
        ) const -> mcs::serialization::OArchive&
      {
        // Deliberately omit the integer expected by the loading constructor.
        return archive;
      }

    private:
      int _value {0};
    };

    struct WaitForMalformedRequest
    {
      using Response = unsigned int;

      std::size_t request_index;
      bool throw_after_resume;
    };

    explicit Handler
      ( std::shared_ptr<RequestsAwaitingMalformedRequest> requests
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

    auto operator() (BrokenSerialization) const -> void
    {
      ADD_FAILURE() << "Malformed command must fail before handler invocation";
    }

    auto operator()
      ( WaitForMalformedRequest request
      ) const -> asio::awaitable<WaitForMalformedRequest::Response>
    {
      co_await _requests->wait_for_malformed_request();

      auto const result {_handler_value + *_argument_value};
      _requests->deliver_result (request.request_index, result);

      if (request.throw_after_resume)
      {
        throw std::runtime_error {"handler failure after malformed request"};
      }

      co_return result;
    }

  private:
    std::shared_ptr<RequestsAwaitingMalformedRequest> _requests;
    mcs::util::not_null<std::promise<void>> _handler_destroyed_promise;
    mcs::util::not_null<unsigned int const> _argument_value;
    unsigned int _handler_value;
  };

  template<mcs::rpc::is_protocol Protocol>
    auto check_malformed_request
      ( unsigned int number_of_threads
      , std::size_t number_of_requests
      ) -> void
  {
    using Dispatcher = mcs::rpc::Dispatcher
                     < Handler
                     , Handler::WaitForMalformedRequest
                     , Handler::BrokenSerialization
                     >;

    constexpr auto timeout {std::chrono::seconds {5}};

    auto random_value {mcs::testing::random::value<unsigned int>{}};
    auto const argument_value {std::invoke (random_value)};
    auto const handler_value {std::invoke (random_value)};
    auto const expected_value {argument_value + handler_value};

    SCOPED_TRACE
      ( ::testing::Message{}
          << "threads: " << number_of_threads
          << ", requests: " << number_of_requests
          << ", argument: " << argument_value
          << ", handler: " << handler_value
      );

    auto handler_destroyed_promise {std::promise<void>{}};
    auto handler_destroyed {handler_destroyed_promise.get_future()};

    auto io_context_provider
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {number_of_threads}
        }
      };
    auto io_context_client
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
          { std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
          }
        }
      };

    auto requests
      { std::make_shared<RequestsAwaitingMalformedRequest>
        ( io_context_provider.get_executor()
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
    auto const broken_archive
      { mcs::serialization::OArchive {Handler::BrokenSerialization {0}}
      };

    mcs::testing::require_exception
      ( [&]
        {
          std::ignore = mcs::serialization::load_from<Handler::BrokenSerialization>
            ( broken_archive.bytes()
            );
        }
      , mcs::testing::assert_type_or_derived_type<mcs::serialization::error::Load>()
      , mcs::testing::assert_type_or_derived_type<mcs::serialization::error::Load>()
      , mcs::testing::assert_type<mcs::serialization::error::NotEnoughBytes>()
      );

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
    responses.reserve (number_of_requests);

    std::generate_n
      ( std::back_inserter (responses)
      , number_of_requests
      , [&client, request {std::size_t {0}}]() mutable
        {
          auto const index {request++};

          return client.get_future
            ( Handler::WaitForMalformedRequest
              { .request_index = index
              , .throw_after_resume = index % 2 != 0
              }
            );
        }
      );

    ASSERT_EQ
      ( requests->wait_for_all_requests_to_be_started (timeout)
      , std::future_status::ready
      );

    auto malformed_response
      { client.get_future (Handler::BrokenSerialization {0})
      };

    ASSERT_EQ (malformed_response.wait_for (timeout), std::future_status::ready);

    mcs::testing::require_exception
      ( [&]
        {
          malformed_response.get();
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
                        EXPECT_EQ (reason.code(), asio::error::eof);
                      }
                    }
                );
            }
          }
      );

    ASSERT_EQ
      ( handler_destroyed.wait_for (std::chrono::seconds {0})
      , std::future_status::timeout
      );

    requests->open_gate();

    EXPECT_TRUE (std::move (*requests).all_results_are_delivered_and_correct());

    ASSERT_EQ (handler_destroyed.wait_for (timeout), std::future_status::ready);
  }

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;

  template<class>
    struct HandlerAfterMalformedRequest : public mcs::testing::random::Test{};

  TYPED_TEST_SUITE (HandlerAfterMalformedRequest, Protocols);
}

TYPED_TEST ( HandlerAfterMalformedRequest
           , malformed_request_keeps_handler_alive_one_thread_one_request
           )
{
  check_malformed_request<TypeParam> (1u, 1u);
}

TYPED_TEST ( HandlerAfterMalformedRequest
           , malformed_request_keeps_handler_alive_one_thread_three_requests
           )
{
  check_malformed_request<TypeParam> (1u, 3u);
}

TYPED_TEST ( HandlerAfterMalformedRequest
           , malformed_request_keeps_handler_alive_four_threads_one_request
           )
{
  check_malformed_request<TypeParam> (4u, 1u);
}

TYPED_TEST ( HandlerAfterMalformedRequest
           , malformed_request_keeps_handler_alive_four_threads_three_requests
           )
{
  check_malformed_request<TypeParam> (4u, 3u);
}

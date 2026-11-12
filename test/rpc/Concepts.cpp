// Copyright (C) 2022,2024-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/access_policy/Concurrent.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/rpc/access_policy/Observed.hpp>
#include <mcs/rpc/access_policy/Sequential.hpp>
#include <mcs/rpc/detail/Completion.hpp>
#include <mcs/rpc/error/Completion.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

namespace mcs::rpc
{
  namespace
  {
    struct Notifications
    {
      int started {0};
      int completed {0};
      int failed {0};
    };

    struct Observer
    {
      explicit Observer (Notifications* notifications_) noexcept
        : notifications {notifications_}
      {}

      auto call_started() noexcept -> void
      {
        ++notifications->started;
      }

      auto call_completed() noexcept -> void
      {
        ++notifications->completed;
      }

      auto error_execution() noexcept -> void
      {
        ++notifications->failed;
      }

      Notifications* notifications;
    };

    template<typename Policy>
      auto check_optional_notifications() -> void
    {
      static_assert (is_access_policy<Policy>);
      static_assert (!has_call_started<Policy>);
      static_assert (!has_call_completed<Policy>);
      static_assert (!has_error_execution<Policy>);
      static_assert
        ( std::is_constructible_v< detail::Completion
                                , std::promise<int>
                                , std::shared_ptr<Policy>
                                >
        );

      auto plain_policy {std::make_shared<Policy>()};
      auto weak_policy {std::weak_ptr<Policy> {plain_policy}};
      auto plain_promise {std::promise<int>{}};
      auto plain_future {plain_promise.get_future()};
      auto plain_completion
        { detail::Completion {std::move (plain_promise), plain_policy}
        };

      plain_policy.reset();
      EXPECT_TRUE (weak_policy.expired());

      auto failure
        { std::make_exception_ptr (std::runtime_error {"test failure"})
        };
      auto const assert_failure
        { testing::Assert<error::Completion>
            { [] (auto const& error)
              {
                ASSERT_STREQ
                  ( error.what()
                  , "rpc::error::Completion: test failure"
                  );

                testing::require_exception
                  ( [&]
                    {
                      std::rethrow_exception (error.reason());
                    }
                  , testing::assert_type_and_what<std::runtime_error>
                      ( "test failure"
                      )
                  );
              }
            }
        };

      plain_completion (failure);

      testing::require_exception
        ( [&]
          {
            plain_future.get();
          }
        , assert_failure
        );

      auto inner_notifications {Notifications{}};
      auto outer_notifications {Notifications{}};

      using Inner = access_policy::Observed<Policy, Observer>;
      using Outer = access_policy::Observed<Inner, Observer>;

      static_assert (is_access_policy<Outer>);
      static_assert (has_call_started<Outer>);
      static_assert (has_call_completed<Outer>);
      static_assert (has_error_execution<Outer>);
      static_assert
        ( std::is_constructible_v< detail::Completion
                                , std::promise<int>
                                , std::shared_ptr<Outer>
                                >
        );
      static_assert (std::same_as<typename Outer::admission, Policy>);

      auto inner
        { std::make_shared<Inner>
          ( std::make_shared<Policy>()
          , std::addressof (inner_notifications)
          )
        };
      auto outer {Outer {inner, std::addressof (outer_notifications)}};

      outer.call_started();
      outer.call_completed();
      outer.call_started();
      outer.error_execution();

      for (auto const* notifications : { std::addressof (inner_notifications)
                                       , std::addressof (outer_notifications)
                                       }
          )
      {
        EXPECT_EQ (notifications->started, 2);
        EXPECT_EQ (notifications->completed, 1);
        EXPECT_EQ (notifications->failed, 1);
      }

      auto promise {std::promise<int>{}};
      auto future {promise.get_future()};
      auto completion {detail::Completion {std::move (promise), inner}};

      EXPECT_EQ (inner_notifications.started, 3);
      EXPECT_EQ (inner_notifications.failed, 1);

      completion (failure);

      EXPECT_EQ (inner_notifications.failed, 2);

      testing::require_exception
        ( [&]
          {
            future.get();
          }
        , assert_failure
        );
    }
  }

  TEST (RPCConcepts, handler_arguments_reject_only_references)
  {
    using FunctionPointer = void (*)();

    static_assert (is_handler_arg<int>);
    static_assert (is_handler_arg<int const>);
    static_assert (is_handler_arg<int*>);
    static_assert (is_handler_arg<int const*>);
    static_assert (is_handler_arg<FunctionPointer>);
    static_assert (is_handler_arg<std::reference_wrapper<int>>);
    static_assert (is_handler_arg<util::not_null<int>>);
    static_assert (is_handler_arg<std::shared_ptr<int>>);
    static_assert (is_handler_arg<std::unique_ptr<int>>);
    static_assert (is_handler_arg<std::span<int>>);
    static_assert (is_handler_arg<std::string_view>);
    static_assert (!is_handler_arg<int&>);
    static_assert (!is_handler_arg<int const&>);
    static_assert (!is_handler_arg<int&&>);
    static_assert (!is_handler_arg<int const&&>);
  }

  TEST ( RPCConcepts
       , access_policies_have_optional_notifications
       )
  {
    check_optional_notifications<access_policy::Exclusive>();
    check_optional_notifications<access_policy::Sequential>();
    check_optional_notifications<access_policy::Concurrent>();
  }

  TEST ( RPCConcepts
       , notification_hooks_are_detected_independently
       )
  {
    struct Started
    {
      auto call_started() noexcept -> void {}
    };

    struct Completed
    {
      auto call_completed() noexcept -> void {}
    };

    struct Failed
    {
      auto error_execution() noexcept -> void {}
    };

    static_assert (has_call_started<Started>);
    static_assert (!has_call_completed<Started>);
    static_assert (!has_error_execution<Started>);
    static_assert (!has_call_started<Completed>);
    static_assert (has_call_completed<Completed>);
    static_assert (!has_error_execution<Completed>);
    static_assert (!has_call_started<Failed>);
    static_assert (!has_call_completed<Failed>);
    static_assert (has_error_execution<Failed>);
  }

  TEST ( RPCConcepts
       , command_without_response_type_is_not_a_command
       )
  {
    struct Command{};
    static_assert (!is_command<Command>);
  }
  TEST ( RPCConcepts
       , command_with_response_is_a_command
       )
  {
    struct Command
    {
      struct Response{};
    };
    static_assert (is_command<Command>);
  }
  TEST ( RPCConcepts
       , command_must_be_serializable
       )
  {
    struct Command
    {
      Command() = delete;
      struct Response{};
    };
    static_assert (!serialization::is_serializable<Command>);
    static_assert (!is_command<Command>);
  }
  TEST ( RPCConcepts
       , response_must_be_serializable
       )
  {
    struct Command
    {
      struct Response
      {
        Response() = delete;
      };
    };
    static_assert (serialization::is_serializable<Command>);
    static_assert (!serialization::is_serializable<Command::Response>);
    static_assert (!is_command<Command>);
  }
  TEST ( RPCConcepts
       , handler_needs_call_operator
       )
  {
    struct Command
    {
      struct Response{};
    };
    struct Handler{};
    static_assert (!is_handler_for_command<Handler, Command>);
  }
  TEST ( RPCConcepts
       , call_operator_from_command_to_response_is_handler_for_command
       )
  {
    struct Command
    {
      struct Response{};
    };
    struct Handler
    {
      constexpr auto operator() (Command) const noexcept -> Command::Response;
    };
    static_assert (is_handler_for_command<Handler, Command>);
  }
  TEST ( RPCConcepts
       , call_operator_from_command_to_awaitable_response_is_handler_for_command
       )
  {
    struct Command
    {
      struct Response{};
    };

    {
      struct HandlerMutable
      {
        constexpr auto operator()
          ( Command
          ) /* mutable */ -> asio::awaitable<Command::Response>;
      };
      static_assert (is_handler_for_command<HandlerMutable, Command>);
    }

    {
      struct HandlerMutableNoexcept
      {
        constexpr auto operator()
          ( Command
          ) /* mutable */ noexcept -> asio::awaitable<Command::Response>;
      };
      static_assert (is_handler_for_command<HandlerMutableNoexcept, Command>);
    }

    {
      struct HandlerConst
      {
        constexpr auto operator()
          ( Command
          ) const -> asio::awaitable<Command::Response>;
      };
      static_assert (is_handler_for_command<HandlerConst, Command>);
    }

    {
      struct HandlerConstNoexcept
      {
        constexpr auto operator()
          ( Command
          ) const noexcept -> asio::awaitable<Command::Response>;
      };
      static_assert (is_handler_for_command<HandlerConstNoexcept, Command>);
    }
  }
}

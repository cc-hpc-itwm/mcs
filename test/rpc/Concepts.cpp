// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <tuple>
#include <utility>

namespace mcs::rpc
{
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

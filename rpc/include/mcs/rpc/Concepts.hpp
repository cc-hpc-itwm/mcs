// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <concepts>
#include <exception>
#include <mcs/rpc/detail/CallID.hpp>
#include <mcs/rpc/detail/Completion.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <type_traits>
#include <utility>

namespace mcs::rpc
{
  template<typename Protocol>
    concept is_protocol = util::ASIO::is_protocol<Protocol>;

  template<typename Protocol, typename Endpoint>
    concept is_endpoint_for_protocol = is_protocol<Protocol>
    && std::is_same_v<Endpoint, typename Protocol::endpoint>
    ;

  template<typename Command>
    concept is_command
     = serialization::is_serializable<Command>
    && (  std::is_same_v<typename Command::Response, void>
       || serialization::is_serializable<typename Command::Response>
       )
      ;

  template<typename Command, typename Socket>
    concept command_is_streaming
      = requires (Command const& command, Socket& socket)
        {
          {command.stream (socket)} -> std::convertible_to<void>;
        };

  template<typename Command, typename... Commands>
    concept is_one_of_the_commands =
      (std::is_same_v<Command, Commands> || ...)
    ;

  namespace handler
  {
    template<typename Handler, typename Command, typename... Args>
      concept provides_response
       = std::invocable<Handler, Command, Args...>
      && std::convertible_to
         < std::invoke_result_t<Handler, Command, Args...>
         , typename Command::Response
         >
    ;
    template<typename Handler, typename Command, typename... Args>
      concept provides_awaitable_response
       = std::invocable<Handler, Command, Args...>
      && std::convertible_to
         < std::invoke_result_t<Handler, Command, Args...>
         , asio::awaitable<typename Command::Response>
         >
    ;
  }

  template<typename Handler, typename Command, typename... Args>
    concept is_handler_for_command =
         handler::provides_response<Handler, Command, Args...>
      || handler::provides_awaitable_response<Handler, Command, Args...>
      ;

  template<typename Handler, typename... Commands>
    concept is_handler_for_commands =
      ( (  is_handler_for_command<Handler, Commands>
        || is_handler_for_command<Handler, Commands, asio::ip::tcp::socket&>
        || is_handler_for_command<Handler, Commands, asio::local::stream_protocol::socket&>
        )
      && ...
      )
    ;

  template<typename AP>
    concept is_access_policy
     = requires ( AP policy
                , detail::CallID call_id
                , detail::Completion completion
                , std::exception_ptr rpc_error
                )
       {
         { policy.start_call (std::move (completion)) } -> std::convertible_to<detail::CallID>;
         { policy.completion (call_id) } -> std::convertible_to<detail::Completion>;
         { policy.error (rpc_error) } -> std::convertible_to<void>;
       }
    ;

  template<typename AP>
    concept needs_read_lock = requires (AP ap) { ap.read_lock(); };
  template<typename AP>
    concept needs_sent_notification = requires (AP ap)
    {
      { ap.sent() } -> std::convertible_to<void>;
    };

  // The observer is informed about the calls into the access policy.
  //
  // The parameters are hidden from the observer -> The access policy
  // can trust that the the parameters are not modifed by the
  // observer(s).
  //
  template<typename Observer>
    concept is_access_policy_observer =
    requires ( Observer observer
             )
    {
      { observer.call_started() } -> std::convertible_to<void>;
      { observer.call_completed() } -> std::convertible_to<void>;
      { observer.error_execution() } -> std::convertible_to<void>;
    };
}

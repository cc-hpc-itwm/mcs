// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <concepts>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <type_traits>
#include <utility>

namespace mcs::rpc
{
  namespace access_policy
  {
    struct Exclusive;
    struct Sequential;
    struct Concurrent;
  }

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
          { command.stream (socket)
          } -> std::convertible_to<void>
          ;
        };

  template<typename Command, typename... Commands>
    concept is_one_of_the_commands =
      ( std::is_same_v<Command, Commands> || ...
      )
    ;

  // Coroutine handler arguments must not be reference types. Value
  // arguments may still borrow and their referents must outlive every
  // operation that uses them.
  //
  template<typename T>
    concept is_handler_arg = !std::is_reference_v<T>;

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

  // Optional policy notification hooks, detected independently so
  // policies may implement only the events they need.
  //
  // - call_started reports submission,
  // - call_completed reports response delivery, including
  //   handler-error responses, and
  // - error_execution reports an RPC/submission failure or rejection,
  //   not a handler error received in a response.
  //
  // The completion hooks call_completed and error_execution run
  // before the future becomes ready. call_completed does not
  // guarantee a successful result: decoding the response or a handler
  // error may still fail the future.
  //
  // call_started runs on the submitting thread. Completion hooks run
  // on the connection strand and synchronous submission failures
  // invoke error_execution on the submitting thread.
  //
  // Hooks must not throw and must tolerate concurrent submissions and
  // completion notifications.
  //
  // Return values are ignored. Observed policies use these hooks to
  // forward events when present.
  //
  template<typename Policy>
    concept has_call_started = requires (Policy& policy)
    {
      policy.call_started();
    };

  template<typename Policy>
    concept has_call_completed = requires (Policy& policy)
    {
      policy.call_completed();
    };

  template<typename Policy>
    concept has_error_execution = requires (Policy& policy)
    {
      policy.error_execution();
    };

  // Policies select a static admission policy tag. The connection
  // owns admission, call IDs, and completions.
  //
  template<typename AP>
    concept is_access_policy
     = requires
       {
         typename AP::admission;
         requires
           (  std::same_as<typename AP::admission, access_policy::Exclusive>
           || std::same_as<typename AP::admission, access_policy::Sequential>
           || std::same_as<typename AP::admission, access_policy::Concurrent>
           );
       }
    ;

  // The observer is informed about the calls into the access policy.
  //
  // The parameters are hidden from the observer -> The access policy
  // can trust that the the parameters are not modifed by the
  // observer(s).
  //
  template<typename Observer>
    concept is_access_policy_observer = requires (Observer observer)
    {
      { observer.call_started() } -> std::convertible_to<void>;
      { observer.call_completed() } -> std::convertible_to<void>;
      { observer.error_execution() } -> std::convertible_to<void>;
    };
}

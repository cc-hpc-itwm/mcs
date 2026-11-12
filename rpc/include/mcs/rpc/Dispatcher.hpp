// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/awaitable.hpp>
#include <cstdint>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/detail/CallID.hpp>
#include <mcs/rpc/detail/CommandIndex.hpp>
#include <mcs/rpc/detail/ReceivedFrame.hpp>
#include <mcs/rpc/detail/ResultHolder.hpp>
#include <mcs/rpc/detail/ResultOrError.hpp>
#include <memory>
#include <type_traits>

namespace mcs::rpc
{
  template<typename Handler, is_command... Commands>
    requires (is_handler_for_commands<Handler, Commands...>)
    struct [[nodiscard]] Dispatcher
  {
    using HandlerType = Handler;

    template<is_protocol Protocol, is_access_policy AccessPolicy>
      using ClientType = Client<Protocol, AccessPolicy, Commands...>;

    struct Header
    {
      detail::CallID call_id;
      detail::CommandIndex index;
    };

    // Both handles retain their owners throughout request execution.
    //
    template<is_protocol Protocol>
      struct RequestContext
    {
      std::shared_ptr<Dispatcher> dispatcher;
      std::shared_ptr<typename Protocol::socket> socket;
    };

    static constexpr auto handshake_data();

    template<typename Command>
      struct IsCommand
    {
      constexpr static auto value {(std::is_same_v<Command, Commands> || ...)};
    };

    template<typename... HandlerArgs>
      requires (std::is_constructible_v<Handler, HandlerArgs...>)
      explicit Dispatcher (HandlerArgs&&...)
        noexcept (std::is_nothrow_constructible_v<Handler, HandlerArgs...>);

    template<is_protocol Protocol>
      [[nodiscard]] static auto dispatch
        ( detail::ReceivedFrame<Header>
        , RequestContext<Protocol>
        ) -> asio::awaitable<detail::ResultHolder>
      ;

  private:
    template<is_protocol Protocol, std::size_t>
      [[nodiscard]] static auto handle_by_index
        ( detail::ReceivedFrame<Header>
        , RequestContext<Protocol>
        ) -> asio::awaitable<detail::ResultHolder>
      ;
    template<is_protocol Protocol, std::size_t, typename, typename...>
      [[nodiscard]] static auto handle_by_index
        ( detail::ReceivedFrame<Header>
        , RequestContext<Protocol>
        ) -> asio::awaitable<detail::ResultHolder>
      ;

    template<is_protocol Protocol, is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
      [[nodiscard]] static auto handle
        ( detail::ReceivedFrame<Header>
        , RequestContext<Protocol>
        ) -> asio::awaitable<detail::ResultHolder>
      ;

    template<is_protocol Protocol, is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
      [[nodiscard]] static auto invoke (Command, RequestContext<Protocol>)
        -> asio::awaitable<detail::ResultOrError<typename Command::Response>>
      ;

    Handler _handler;
  };
}

#include "detail/Dispatcher.ipp"

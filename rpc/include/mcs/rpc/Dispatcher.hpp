// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/awaitable.hpp>
#include <cstdint>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/detail/Buffer.hpp>
#include <mcs/rpc/detail/CallID.hpp>
#include <mcs/rpc/detail/CommandIndex.hpp>
#include <mcs/rpc/detail/ResultHolder.hpp>
#include <mcs/rpc/detail/ResultOrError.hpp>
#include <tuple>
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

    static constexpr auto handshake_data();

    template<typename Command>
      struct IsCommand
    {
      constexpr static auto value {(std::is_same_v<Command, Commands> || ...)};
    };

    template<typename... HandlerArgs>
      requires (std::is_constructible_v<Handler, HandlerArgs...>)
      explicit Dispatcher (HandlerArgs&&...);

    template<is_protocol Protocol>
      auto dispatch
        ( std::tuple<Header, detail::Buffer>
        , typename Protocol::socket&
        ) -> asio::awaitable<detail::ResultHolder>
      ;

  private:
    template<is_protocol Protocol, std::size_t>
      auto handle_by_index
        ( std::tuple<Header, detail::Buffer>
        , typename Protocol::socket&
        ) -> asio::awaitable<detail::ResultHolder>
      ;
    template<is_protocol Protocol, std::size_t, typename, typename...>
      auto handle_by_index
        ( std::tuple<Header, detail::Buffer>
        , typename Protocol::socket&
        ) -> asio::awaitable<detail::ResultHolder>
      ;

    template<is_protocol Protocol, is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
      auto handle
        ( std::tuple<Header, detail::Buffer>
        , typename Protocol::socket&
        ) -> asio::awaitable<detail::ResultHolder>
      ;

    template<is_protocol Protocol, is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
      [[nodiscard]] auto invoke (Command, typename Protocol::socket&)
        -> asio::awaitable<detail::ResultOrError<typename Command::Response>>
      ;

    Handler _handler;
  };
}

#include "detail/Dispatcher.ipp"

// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/awaitable.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/util/ASIO/ListeningAcceptor.hpp>
#include <type_traits>

namespace mcs::rpc
{
  template<is_protocol Protocol, typename Dispatcher, typename... HandlerArgs>
    struct [[nodiscard]] Provider
  {
    static_assert
      (std::is_constructible_v<typename Dispatcher::HandlerType, HandlerArgs...>);

    auto local_endpoint() const;

    template<typename Executor>
      explicit Provider
        ( typename Protocol::endpoint
        , Executor&
        , HandlerArgs...
        );

  private:
    util::ASIO::ListeningAcceptor<Protocol> _acceptor;

    auto accept_clients
      ( HandlerArgs...
      ) -> asio::awaitable<void>;
    auto dispatch
      ( typename Protocol::socket
      , HandlerArgs...
      ) -> asio::awaitable<void>;
  };

  template< is_protocol Protocol
          , typename Dispatcher
          , typename Executor
          , typename... HandlerArgs
          >
    requires (std::is_constructible_v<typename Dispatcher::HandlerType, HandlerArgs...>)
      auto make_provider
        ( typename Protocol::endpoint
        , Executor&
        , HandlerArgs...
        ) -> Provider<Protocol, Dispatcher, HandlerArgs...>
    ;
}

#include "detail/Provider.ipp"

// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/any_io_executor.hpp>
#include <asio/awaitable.hpp>
#include <asio/error_code.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>

namespace mcs::util::ASIO
{
  template<is_protocol Protocol>
    struct ListeningAcceptor
  {
    template<typename Executor>
      ListeningAcceptor
        ( Executor&
        , typename Protocol::endpoint
        );

    auto local_endpoint() const -> typename Protocol::endpoint;
    auto accept() -> typename Protocol::socket;
    auto async_accept() -> asio::awaitable<typename Protocol::socket>;
    [[nodiscard]] auto async_accept
      ( asio::any_io_executor
      ) -> asio::awaitable<typename Protocol::socket>;
    auto close (asio::error_code&) -> void;

    // \note Protocol::acceptor _is_ moveable but to move it leads to crashes
    ListeningAcceptor (ListeningAcceptor const&) = delete;
    ListeningAcceptor (ListeningAcceptor&&) = delete;
    auto operator= (ListeningAcceptor&&) -> ListeningAcceptor& = delete;
    auto operator= (ListeningAcceptor const&) -> ListeningAcceptor& = delete;
    ~ListeningAcceptor() = default;

  private:
    typename Protocol::acceptor _acceptor;
  };
}

#include "detail/ListeningAcceptor.ipp"

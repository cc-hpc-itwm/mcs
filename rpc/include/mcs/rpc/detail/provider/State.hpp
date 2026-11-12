// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/any_io_executor.hpp>
#include <asio/error_code.hpp>
#include <asio/steady_timer.hpp>
#include <asio/strand.hpp>
#include <atomic>
#include <chrono>
#include <concepts>
#include <exception>
#include <future>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/util/ASIO/ListeningAcceptor.hpp>
#include <memory>

namespace mcs::rpc
{
  template< is_protocol Protocol
          , typename Dispatcher
          , is_handler_arg... HandlerArgs
          >
    struct Provider;
}

namespace mcs::rpc::detail::provider
{
  namespace detail
  {
    // Stored dispatches are called as lvalues with a moved socket.
    //
    template<typename Dispatch, typename Protocol>
      concept is_dispatch = util::ASIO::is_protocol<Protocol>
      && std::move_constructible<Dispatch>
      && std::invocable<Dispatch&, typename Protocol::socket>
      ;

    template<util::ASIO::is_protocol Protocol>
      struct AcceptClients;
  }

  template<util::ASIO::is_protocol Protocol>
    struct State : public std::enable_shared_from_this<State<Protocol>>
  {
    template<typename Executor>
      explicit State (Executor&, typename Protocol::endpoint);

  private:
    template< is_protocol OtherProtocol
            , typename Dispatcher
            , is_handler_arg... HandlerArgs
            >
      friend struct mcs::rpc::Provider;

    friend struct detail::AcceptClients<Protocol>;

    template<detail::is_dispatch<Protocol> Dispatch>
      auto start (Dispatch) -> void;

    [[nodiscard]] auto executor() const -> asio::any_io_executor;
    [[nodiscard]] auto local_endpoint() const -> typename Protocol::endpoint;
    [[nodiscard]] auto stop_accepting() -> std::shared_future<void>;
    auto request_stop() -> void;

    static constexpr auto _diagnostic_interval
      { std::chrono::milliseconds {333}
      };

    asio::strand<asio::any_io_executor> _strand;
    util::ASIO::ListeningAcceptor<Protocol> _acceptor;
    asio::steady_timer _retry_timer {_strand};
    typename Protocol::endpoint _endpoint {_acceptor.local_endpoint()};
    std::atomic_bool _stop_requested {false};
    bool _finished {false};
    asio::error_code _close_error;
    std::promise<void> _stopped;
    std::shared_future<void> _stopped_future {_stopped.get_future().share()};

    auto close() -> void;
    auto finish (std::exception_ptr) -> void;
  };
}

#include "detail/State.ipp"

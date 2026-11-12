// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <future>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/detail/provider/State.hpp>
#include <memory>
#include <type_traits>

namespace mcs::rpc
{
  template< is_protocol Protocol
          , typename Dispatcher
          , is_handler_arg... HandlerArgs
          >
    struct [[nodiscard]] Provider
  {
    static_assert
      ( std::is_constructible_v< typename Dispatcher::HandlerType
                               , HandlerArgs...
                               >
      );

    [[nodiscard]] auto local_endpoint() const -> typename Protocol::endpoint;

    // Stops the listener, not established connections or their
    // handlers. The context must outlive the provider and run until
    // completion. Do not wait on its worker thread. get() reports
    // terminal loop errors.
    //
    [[nodiscard]] auto stop_accepting() const -> std::shared_future<void>;

    template<typename Executor>
      explicit Provider
        ( typename Protocol::endpoint
        , Executor&
          // \todo deal with handler arg copies precisly
        , HandlerArgs...
        );

    // Requests stopping without waiting. Use stop_accepting() to
    // await close.
    //
    ~Provider();
    Provider (Provider const&) = delete;
    Provider (Provider&&) = delete;
    auto operator= (Provider const&) -> Provider& = delete;
    auto operator= (Provider&&) -> Provider& = delete;

  private:
    std::shared_ptr<detail::provider::State<Protocol>> _state;
  };

  template< is_protocol Protocol
          , typename Dispatcher
          , typename Executor
          , is_handler_arg... HandlerArgs
          >
    requires ( std::is_constructible_v< typename Dispatcher::HandlerType
                                      , HandlerArgs...
                                      >
             )
      auto make_provider
        ( typename Protocol::endpoint
        , Executor&
        , HandlerArgs...
        ) -> Provider<Protocol, Dispatcher, HandlerArgs...>
    ;
}

#include "detail/Provider.ipp"

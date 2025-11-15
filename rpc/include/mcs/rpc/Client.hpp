// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <functional>
#include <future>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/access_policy/Concurrent.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/rpc/access_policy/Sequential.hpp>
#include <mcs/rpc/detail/ClientState.hpp>
#include <mcs/rpc/detail/CommandIndex.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <type_traits>

namespace mcs::rpc
{
  // Access managed by the application, at most one concurrent command
  // at client side, at most one concurrent command per client at
  // server side.
  //
  // Costs: No lock. No storage. Cheapest.
  //
  // The asynchronous calls are never blocking, the synchronous calls
  // are always blocking. The application must ensure there are no
  // concurrent calls! Note: NO CONCURRENT CALLS! That includes all
  // asynchrounous and synchronous calls.
  //
  static_assert (is_access_policy<access_policy::Exclusive>);

  // Access sequentialized by the rpc client, multiple concurrent
  // commands at client side are executed in sequence in unspecified
  // order, at most one concurrent command per client at server side.
  //
  // Costs: One local lock per remote call. No storage.
  //
  // The asynchronous calls may block, the sychronous calls are always
  // blocking. The application may do concurrent calls. The Client
  // makes sure only one call enters the critical section at the same
  // time. The order of execution of multiple calls is not
  // specified. The result of a late synchronous or asynchronous call
  // can be requested and might be available before the result of an
  // early asynchronous call has been retrieved.
  //
  static_assert (is_access_policy<access_policy::Sequential>);

  // Multiple concurrent commands at client side are executed in
  // parallel in unspecified order. Multiple concurrent commands per
  // client may reach the server side.
  //
  // Costs:
  // - some locks per remote call.
  // - Space: O(number_of_parallel_operations)
  // - Time: O(log (number_of_parallel_operations)) per remote call
  //
  // The asynchronous calls are never blocking, the synchronous
  // calls are always blocking. The order in which the calls arrive
  // at the server is not specified. The result of a late
  // synchronous or asynchronous call can be requested and might be
  // available before the result of an early asynchronous call has
  // been retrieved.
  //
  static_assert (is_access_policy<access_policy::Concurrent>);

  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    struct Client
  {
    //! \todo timeout

    // deliver the command by reference, make sure the command
    // outlives the call completion
    //
    template<is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
        auto get_future
          ( std::reference_wrapper<Command const>
          ) const -> std::future<typename Command::Response>;

    template<is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
        auto operator()
          ( std::reference_wrapper<Command const>
          ) const -> typename Command::Response;


    // deliver the command by constructor arguments, the command is
    // created inside the client
    //
    template<is_command Command, typename... CommandArgs>
      requires ( std::is_constructible_v<Command, CommandArgs...>
              && is_one_of_the_commands<Command, Commands...>
               )
      auto async_call
        ( CommandArgs&&...
        ) const -> std::future<typename Command::Response>;

    template<is_command Command, typename... CommandArgs>
      requires ( std::is_constructible_v<Command, CommandArgs...>
              && is_one_of_the_commands<Command, Commands...>
               )
      auto call
        ( CommandArgs&&...
        ) const -> typename Command::Response;

    // deliver the command as rvalue, the command is forwarded into
    // the client
    //
    template<is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
      auto get_future
        ( Command&&
        ) const -> std::future<typename Command::Response>;

    template<is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
      auto operator()
        ( Command&&
        ) const -> typename Command::Response;

    // Produces a new client that adds an Observer to the access policy.
    //
    template<is_access_policy_observer Observer, typename... ObserverArgs>
      requires (std::is_constructible_v<Observer, ObserverArgs...>)
      [[nodiscard]] constexpr auto observe
        ( ObserverArgs&&...
        ) const noexcept (std::is_nothrow_constructible_v<Observer, ObserverArgs...>)
      ;

    // Construction, use make_client
    //
    template<typename Executor>
      explicit Client
        ( Executor&
        , typename Protocol::endpoint
        , std::shared_ptr<AccessPolicy>
        );

    template<typename Executor>
      explicit Client
        ( Executor&
        , util::ASIO::Connectable<Protocol>
        , std::shared_ptr<AccessPolicy>
        );

    constexpr explicit Client
      ( detail::ClientState<Protocol, AccessPolicy, Commands...>
      ) noexcept;

  private:
    detail::ClientState<Protocol, AccessPolicy, Commands...> _state;
  };

  template< is_protocol Protocol
          , typename Dispatcher
          , is_access_policy AccessPolicy
          , typename Executor
          >
    requires (std::is_default_constructible_v<AccessPolicy>)
    auto make_client
      ( Executor&
      , typename Protocol::endpoint
      );

  template< is_protocol Protocol
          , typename Dispatcher
          , is_access_policy AccessPolicy
          , typename Executor
          >
    requires (std::is_default_constructible_v<AccessPolicy>)
    auto make_client
      ( Executor&
      , typename util::ASIO::Connectable<Protocol>
      );
}

#include "detail/Client.ipp"

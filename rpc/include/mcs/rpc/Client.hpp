// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <functional>
#include <future>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/access_policy/Concurrent.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/rpc/access_policy/Sequential.hpp>
#include <mcs/rpc/detail/CommandIndex.hpp>
#include <mcs/rpc/detail/client/Connection.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <memory>
#include <type_traits>

namespace mcs::rpc
{
  // Access managed by the application, at most one concurrent command
  // at client side, at most one concurrent command per client at
  // server side.
  //
  // Costs: One pending-call slot, no waiting queue or call-ID map.
  //
  // The asynchronous calls are never blocking, the synchronous calls
  // are always blocking. The application must ensure there are no
  // concurrent calls! Note: NO CONCURRENT CALLS! That includes all
  // asynchronous and synchronous calls.
  //
  static_assert (is_access_policy<access_policy::Exclusive>);

  // Access sequentialized by the rpc client, multiple concurrent
  // commands at client side are executed in sequence in unspecified
  // order, at most one concurrent command per client at server side.
  //
  // Costs: One active-call slot and a queue for waiting calls.
  //
  // Asynchronous calls enqueue without waiting for earlier
  // calls. Synchronous calls wait for their own result. The next
  // request is admitted only after the previous response is
  // handled. Simultaneous callers have unspecified ordering.
  // Sequential submissions preserve their order. Retrieving a future
  // is not required to admit the next request.
  //
  static_assert (is_access_policy<access_policy::Sequential>);

  // Multiple concurrent commands at client side are executed in
  // parallel in unspecified order. Multiple concurrent commands per
  // client may reach the server side.
  //
  // Costs:
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

  // All policies use a shared connection that serializes asynchronous
  // sends and owns a single response reader. Serialization,
  // transport, and streaming errors reach the call's
  // future. Construction and handshake still run synchronously, as
  // does construction of commands passed to async_call(). Allocation
  // and submission errors may throw directly. Synchronous calls
  // rethrow errors while waiting for their result. A synchronous
  // command.stream (socket) hook runs on the connection's executor
  // and must return before shutdown can finish. The shared
  // connection, strand, and coroutine scheduling are common costs.
  // Client handles retain their policy and the shared connection;
  // observed handles wrap the policy without creating a new connection.

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
        [[nodiscard]] auto get_future
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
      [[nodiscard]] auto async_call
        ( CommandArgs&&...
        ) const -> std::future<typename Command::Response>;

    template<is_command Command, typename... CommandArgs>
      requires ( std::is_constructible_v<Command, CommandArgs...>
              && is_one_of_the_commands<Command, Commands...>
               )
      [[nodiscard]] auto call
        ( CommandArgs&&...
        ) const -> typename Command::Response;

    // deliver the command as rvalue, the command is forwarded into
    // the client
    //
    template<is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
      [[nodiscard]] auto get_future
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

    // Closes the shared connection and fails outstanding calls. The
    // executor must keep running until the returned future is
    // ready. Do not wait for that future on an IO worker. Destroying
    // a client handle alone does not cancel its calls. Copies and
    // observed clients share shutdown. Repeated shutdown is safe and
    // later calls fail with the connection's error. An open
    // connection is stopped with error::Completion whose reason is
    // operation_aborted. Existing failures retain their
    // reason. Read/write completions are drained before the future
    // becomes ready. This does not cancel remote handlers or shut
    // down the provider. Destroying the execution context instead of
    // draining shutdown abandons pending work.
    //
    [[nodiscard]] auto shutdown() const -> std::future<void>;

  private:
    using Connection = detail::client::Connection<Protocol, typename AccessPolicy::admission>;

    template<is_protocol, is_access_policy, is_command...>
      friend struct Client;

    constexpr explicit Client
      ( std::shared_ptr<AccessPolicy>
      , std::shared_ptr<Connection>
      ) noexcept;

    std::shared_ptr<AccessPolicy> _access_policy;
    std::shared_ptr<Connection> _connection;
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

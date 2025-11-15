// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/detail/command_holder/NonOwning.hpp>
#include <mcs/rpc/detail/command_holder/Owning.hpp>
#include <mcs/rpc/detail/observe.hpp>
#include <mcs/rpc/detail/remote_call.hpp>
#include <type_traits>

namespace mcs::rpc
{
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    constexpr Client<Protocol, AccessPolicy, Commands...>::Client
      ( detail::ClientState<Protocol, AccessPolicy, Commands...> state
      ) noexcept
        : _state {state}
  {}

  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<typename Executor>
      Client<Protocol, AccessPolicy, Commands...>::Client
        ( Executor& executor
        , typename Protocol::endpoint endpoint
        , std::shared_ptr<AccessPolicy> access_policy
        )
          : Client<Protocol, AccessPolicy, Commands...>
            { detail::ClientState<Protocol, AccessPolicy, Commands...>
                { executor
                , endpoint
                , access_policy
                }
            }
  {}

  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<typename Executor>
      Client<Protocol, AccessPolicy, Commands...>::Client
        ( Executor& executor
        , util::ASIO::Connectable<Protocol> connectable
        , std::shared_ptr<AccessPolicy> access_policy
        )
          : Client<Protocol, AccessPolicy, Commands...>
            { detail::ClientState<Protocol, AccessPolicy, Commands...>
                { executor
                , connectable
                , access_policy
                }
            }
  {}

  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    auto make_client
      ( detail::ClientState<Protocol, AccessPolicy, Commands...> state
      ) noexcept
  {
    return Client<Protocol, AccessPolicy, Commands...> {state};
  }

  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<is_access_policy_observer Observer, typename... ObserverArgs>
      requires (std::is_constructible_v<Observer, ObserverArgs...>)
      constexpr auto Client<Protocol, AccessPolicy, Commands...>::observe
        ( ObserverArgs&&... observer_args
        ) const noexcept (std::is_nothrow_constructible_v<Observer, ObserverArgs...>)
  {
    return make_client
      ( detail::observe<Observer>
          (_state, std::forward<ObserverArgs> (observer_args)...)
      );
  }
}

// deliver the command by reference
//
namespace mcs::rpc
{
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
        auto Client<Protocol, AccessPolicy, Commands...>::get_future
          ( std::reference_wrapper<Command const> command_ref
          ) const -> std::future<typename Command::Response>
  {
    return detail::remote_call<Protocol, AccessPolicy, Command>
      ( _state
      , detail::command_holder::NonOwning<Command> {command_ref}
      );
  }
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
        auto Client<Protocol, AccessPolicy, Commands...>::operator()
          ( std::reference_wrapper<Command const> command_ref
          ) const -> typename Command::Response
  {
    return get_future (command_ref).get();
  }
}

// deliver the command by constructor arguments
//
namespace mcs::rpc
{
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<is_command Command, typename... CommandArgs>
      requires ( std::is_constructible_v<Command, CommandArgs...>
              && is_one_of_the_commands<Command, Commands...>
               )
        auto Client<Protocol, AccessPolicy, Commands...>::async_call
          ( CommandArgs&&... command_args
          ) const -> std::future<typename Command::Response>
  {
    return detail::remote_call<Protocol, AccessPolicy, Command>
      ( _state
      , detail::command_holder::Owning<Command>
          {std::forward<CommandArgs> (command_args)...}
      );
  }
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<is_command Command, typename... CommandArgs>
      requires ( std::is_constructible_v<Command, CommandArgs...>
              && is_one_of_the_commands<Command, Commands...>
               )
        auto Client<Protocol, AccessPolicy, Commands...>::call
          ( CommandArgs&&... command_args
          ) const -> typename Command::Response
  {
    return async_call<Command> (std::forward<CommandArgs> (command_args)...).get();
  }
}

// deliver the command as rvalue
//
namespace mcs::rpc
{
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
        auto Client<Protocol, AccessPolicy, Commands...>::get_future
          ( Command&& command
          ) const -> std::future<typename Command::Response>
  {
    return detail::remote_call<Protocol, AccessPolicy, Command>
      ( _state
      , detail::command_holder::Owning<Command>
          {std::forward<Command> (command)}
      );
  }
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
        auto Client<Protocol, AccessPolicy, Commands...>::operator()
          ( Command&& command
          ) const -> typename Command::Response
  {
    return get_future (std::forward<Command> (command)).get();
  }
}

namespace mcs::rpc
{
  template< is_protocol Protocol
          , typename Dispatcher
          , is_access_policy AccessPolicy
          , typename Executor
          >
    requires (std::is_default_constructible_v<AccessPolicy>)
    auto make_client
      ( Executor& executor
      , typename Protocol::endpoint endpoint
      )
  {
    return make_client<Protocol, Dispatcher, AccessPolicy>
      ( executor
      , endpoint
      , std::make_shared<AccessPolicy>()
      );
  }

  template< is_protocol Protocol
          , typename Dispatcher
          , is_access_policy AccessPolicy
          , typename Executor
          >
    requires (std::is_default_constructible_v<AccessPolicy>)
    auto make_client
      ( Executor& executor
      , typename util::ASIO::Connectable<Protocol> connectable
      )
  {
    return make_client<Protocol, Dispatcher, AccessPolicy>
      ( executor
      , connectable
      , std::make_shared<AccessPolicy>()
      );
  }
}

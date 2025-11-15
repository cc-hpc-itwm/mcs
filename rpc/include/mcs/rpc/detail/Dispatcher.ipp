// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdint>
#include <exception>
#include <functional>
#include <mcs/rpc/detail/make_handshake_data.hpp>
#include <mcs/rpc/error/internal/UnknownCommand.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <utility>

namespace mcs::rpc
{
  template<typename Handler, is_command... Commands>
    requires (is_handler_for_commands<Handler, Commands...>)
    template<typename... HandlerArgs>
      requires (std::is_constructible_v<Handler, HandlerArgs...>)
      Dispatcher<Handler, Commands...>::Dispatcher
        ( HandlerArgs&&... handler_args
        )
          : _handler {std::forward<HandlerArgs> (handler_args)...}
  {}


  template<typename Handler, is_command... Commands>
    requires (is_handler_for_commands<Handler, Commands...>)
    constexpr auto Dispatcher<Handler, Commands...>::handshake_data
      (
      )
  {
    return detail::make_handshake_data<Commands...>();
  }

  template<typename Handler, is_command... Commands>
    requires (is_handler_for_commands<Handler, Commands...>)
    template<is_protocol Protocol>
      auto Dispatcher<Handler, Commands...>::dispatch
        ( std::tuple<Header, detail::Buffer> command
        , typename Protocol::socket& socket
        ) -> asio::awaitable<detail::ResultHolder>
  {
    return handle_by_index<Protocol, 0, Commands...>
      (std::move (command), socket);
  }

  template<typename Handler, is_command... Commands>
    requires (is_handler_for_commands<Handler, Commands...>)
    template<is_protocol Protocol, std::size_t I>
      auto Dispatcher<Handler, Commands...>::handle_by_index
        ( std::tuple<Header, detail::Buffer>
        , typename Protocol::socket&
        ) -> asio::awaitable<detail::ResultHolder>
  {
    throw error::internal::UnknownCommand{};
  }

  template<typename Handler, is_command... Commands>
    requires (is_handler_for_commands<Handler, Commands...>)
    template< is_protocol Protocol
            , std::size_t I
            , typename Command
            , typename... Tail
            >
      auto Dispatcher<Handler, Commands...>::handle_by_index
        ( std::tuple<Header, detail::Buffer> command
        , typename Protocol::socket& socket
        ) -> asio::awaitable<detail::ResultHolder>
  {
    auto& header {std::get<Header> (command)};

    if (header.index == detail::CommandIndex{})
    {
      return handle<Protocol, Command> (std::move (command), socket);
    }
    else
    {
      --header.index;

      return handle_by_index<Protocol, I + 1, Tail...>
        (std::move (command), socket);
    }
  }

  template<typename Handler, is_command... Commands>
    requires (is_handler_for_commands<Handler, Commands...>)
    template<is_protocol Protocol, is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
      auto Dispatcher<Handler, Commands...>::handle
        ( std::tuple<Header, detail::Buffer> command
        , typename Protocol::socket& socket
        ) -> asio::awaitable<detail::ResultHolder>
  {
    auto const& header {std::get<Header> (command)};
    auto& buffer {std::get<detail::Buffer> (command)};

    auto result_or_error
      {co_await invoke<Protocol> (buffer.template load<Command>(), socket)};

    co_return detail::ResultHolder
      {header.call_id, std::move (result_or_error)};
  }

  template<typename Handler, is_command... Commands>
    requires (is_handler_for_commands<Handler, Commands...>)
    template<is_protocol Protocol, is_command Command>
      requires (is_one_of_the_commands<Command, Commands...>)
      auto Dispatcher<Handler, Commands...>::invoke
        ( Command command
        , typename Protocol::socket& socket
        ) -> asio::awaitable<detail::ResultOrError<typename Command::Response>>
  try
  {
    using Socket = typename Protocol::socket&;

    if constexpr (handler::provides_response<Handler, Command, Socket>)
    {
      if constexpr (std::is_same_v<typename Command::Response, void>)
      {
        std::invoke (_handler, std::move (command), socket);

        co_return detail::make_result();
      }
      else
      {
        co_return detail::make_result
          ( std::invoke (_handler, std::move (command), socket)
          );
      }
    }

    if constexpr (handler::provides_awaitable_response<Handler, Command, Socket>)
    {
      if constexpr (std::is_same_v<typename Command::Response, void>)
      {
        co_await std::invoke (_handler, std::move (command), socket);

        co_return detail::make_result();
      }
      else
      {
        co_return detail::make_result
          ( co_await std::invoke (_handler, std::move (command), socket)
          );
      }
    }

    if constexpr (handler::provides_response<Handler, Command>)
    {
      if constexpr (std::is_same_v<typename Command::Response, void>)
      {
        std::invoke (_handler, std::move (command));

        co_return detail::make_result();
      }
      else
      {
        co_return detail::make_result
          ( std::invoke (_handler, std::move (command))
          );
      }
    }

    if constexpr (handler::provides_awaitable_response<Handler, Command>)
    {
      if constexpr (std::is_same_v<typename Command::Response, void>)
      {
        co_await std::invoke (_handler, std::move (command));

        co_return detail::make_result();
      }
      else
      {
        co_return detail::make_result
          ( co_await std::invoke (_handler, std::move (command))
          );
      }
    }
  }
  catch (...)
  {
    co_return detail::make_error<typename Command::Response>
      (fmt::format ("{}", std::current_exception()));
  }

  template< is_protocol Protocol
          , typename Dispatcher
          , is_access_policy AccessPolicy
          , typename Executor
          >
    auto make_client
      ( Executor& executor
      , typename Protocol::endpoint endpoint
      , std::shared_ptr<AccessPolicy> access_policy
      )
  {
    return typename Dispatcher::template ClientType<Protocol, AccessPolicy>
      { executor
      , endpoint
      , access_policy
      };
  }

  template< is_protocol Protocol
          , typename Dispatcher
          , is_access_policy AccessPolicy
          , typename Executor
          >
    auto make_client
      ( Executor& executor
      , util::ASIO::Connectable<Protocol> connectable
      , std::shared_ptr<AccessPolicy> access_policy
      )
  {
    return typename Dispatcher::template ClientType<Protocol, AccessPolicy>
      { executor
      , connectable
      , access_policy
      };
  }
}

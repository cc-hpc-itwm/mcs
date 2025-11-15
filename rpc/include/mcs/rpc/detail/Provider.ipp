// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/bind_executor.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/strand.hpp>
#include <asio/this_coro.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>
#include <exception>
#include <mcs/rpc/detail/Buffer.hpp>
#include <mcs/rpc/detail/ResultHolder.hpp>
#include <mcs/rpc/detail/receive_buffer_with_header.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/util/ASIO/SetSocketOptions.hpp>
#include <memory>
#include <utility>

namespace mcs::rpc
{
  template<is_protocol Protocol, typename Dispatcher, typename... HandlerArgs>
    template<typename Executor>
      Provider<Protocol, Dispatcher, HandlerArgs...>::Provider
        ( typename Protocol::endpoint endpoint
        , Executor& executor
        , HandlerArgs... handler_args
        )
          : _acceptor {executor, endpoint}
  {
    asio::co_spawn
      ( executor
      , accept_clients (handler_args...)
      , asio::detached
      );
  }

  template<is_protocol Protocol, typename Dispatcher, typename... HandlerArgs>
    auto Provider<Protocol, Dispatcher, HandlerArgs...>::local_endpoint
      (
      ) const
  {
    return _acceptor.local_endpoint();
  }

  template<is_protocol Protocol, typename Dispatcher, typename... HandlerArgs>
    auto Provider<Protocol, Dispatcher, HandlerArgs...>::accept_clients
      ( HandlerArgs... handler_args
      ) -> asio::awaitable<void>
  {
    auto executor {co_await asio::this_coro::executor};

    while (true)
    {
      auto socket {co_await _acceptor.async_accept()};

      asio::co_spawn
        ( executor
        , dispatch (std::move (socket), handler_args...)
        , asio::detached
        );
    }
  }

  template<is_protocol Protocol, typename Dispatcher, typename... HandlerArgs>
    auto Provider<Protocol, Dispatcher, HandlerArgs...>::dispatch
      ( typename Protocol::socket socket
      , HandlerArgs... handler_args
      ) -> asio::awaitable<void>
  try
  {
    util::ASIO::SetSocketOptions<Protocol>{} (socket);

    {
      auto const data {Dispatcher::handshake_data()};
      auto const oa {serialization::OArchive {data}};

      co_await asio::async_write (socket, oa.buffers(), asio::use_awaitable);
    }

    auto executor {co_await asio::this_coro::executor};
    auto strand {asio::make_strand (executor)};
    auto dispatcher {Dispatcher {handler_args...}};

    auto error {std::exception_ptr{}};

    while (!error)
    {
      auto response
        {co_await detail::receive_buffer_with_header<typename Dispatcher::Header> (socket)};

      asio::co_spawn
        ( executor
        , dispatcher.template dispatch<Protocol> (std::move (response), socket)
        , asio::bind_executor
          ( strand
          , [&]
              ( std::exception_ptr rpc_error
              , detail::ResultHolder result_holder
              ) noexcept
            {
              if (rpc_error)
              {
                error = rpc_error;
              }
              else
              {
                // \note the write might fail in case that the client
                // has been destroyed, exceptions must be catched to
                // prevent from terminate as the completion handler
                // must be noexcept
                try
                {
                  asio::write (socket, result_holder.archive().buffers());
                }
                catch (...)
                {
                  error = std::current_exception();
                }
              }
            }
          )
        );
    }

    // \todo handle rpc_error in a specific way rethrow and catch
    // below std::rethrow_exception (error);
  }
  catch (...) // NOLINT (bugprone-empty-catch)
  {
    // go home, the client has gone, somehow
  }
}

namespace mcs::rpc
{
  template< is_protocol Protocol
          , typename Dispatcher
          , typename Executor
          , typename... HandlerArgs
          >
    requires (std::is_constructible_v<typename Dispatcher::HandlerType, HandlerArgs...>)
    auto make_provider
      ( typename Protocol::endpoint endpoint
      , Executor& executor
      , HandlerArgs... handler_args
      ) -> Provider<Protocol, Dispatcher, HandlerArgs...>
  {
    return Provider<Protocol, Dispatcher, HandlerArgs...>
      { endpoint
      , executor
      , handler_args...
      };
  }
}

// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/bind_executor.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/strand.hpp>
#include <asio/this_coro.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>
#include <atomic>
#include <cstdio>
#include <exception>
#include <functional>
#include <mcs/rpc/detail/Buffer.hpp>
#include <mcs/rpc/detail/ResultHolder.hpp>
#include <mcs/rpc/detail/receive_buffer_with_header.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/util/ASIO/SetSocketOptions.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mcs::rpc::detail
{
  template< is_protocol Protocol
          , typename Dispatcher
          , is_handler_arg... HandlerArgs
          >
    struct ConnectionState
  {
    explicit ConnectionState
      ( typename Protocol::socket socket
      , HandlerArgs... handler_args
      ) noexcept
          (  std::is_nothrow_move_constructible_v<typename Protocol::socket>
          && std::is_nothrow_constructible_v<Dispatcher, HandlerArgs...>
          )
        : _socket {std::move (socket)}
        , _dispatcher {std::forward<HandlerArgs> (handler_args)...}
    {}

    [[nodiscard]] auto socket() -> typename Protocol::socket&
    {
      return _socket;
    }

    [[nodiscard]] auto ok() const noexcept -> bool
    {
      return !_closed.load();
    }

    [[nodiscard]] static auto invoke
      ( std::shared_ptr<ConnectionState> state
      , ReceivedFrame<typename Dispatcher::Header> response
      ) -> asio::awaitable<detail::ResultHolder>
    {
      return Dispatcher::template dispatch<Protocol>
        ( std::move (response)
        , typename Dispatcher::template RequestContext<Protocol>
          { .dispatcher = std::shared_ptr<Dispatcher>
              { state
              , std::addressof (state->_dispatcher)
              }
          , .socket = std::shared_ptr<typename Protocol::socket>
              { state
              , std::addressof (state->_socket)
              }
          }
        );
    }

    auto shutdown() noexcept -> void
    {
      if (!_closed.exchange (true))
      {
        auto error {asio::error_code{}};

        _socket.shutdown (Protocol::socket::shutdown_both, error);
      }
    }

  private:
    typename Protocol::socket _socket;
    Dispatcher _dispatcher;
    std::atomic_bool _closed {false};
  };

  template< is_protocol Protocol
          , typename Dispatcher
          , is_handler_arg... HandlerArgs
          >
    auto dispatch
      ( typename Protocol::socket socket
      , HandlerArgs... handler_args
      ) noexcept -> asio::awaitable<void>
  try
  {
    std::invoke (util::ASIO::SetSocketOptions<Protocol>{}, socket);

    {
      auto const data {Dispatcher::handshake_data()};
      auto const oa {serialization::OArchive {data}};

      co_await asio::async_write (socket, oa.buffers(), asio::use_awaitable);
    }

    auto executor {co_await asio::this_coro::executor};
    auto strand {asio::make_strand (executor)};
    auto state
      { std::make_shared<ConnectionState<Protocol, Dispatcher, HandlerArgs...>>
          ( std::move (socket)
          , std::forward<HandlerArgs> (handler_args)...
          )
      };
    auto const owned_socket
      { std::shared_ptr<typename Protocol::socket>
          { state
          , std::addressof (state->socket())
          }
      };

    try
    {
      while (state->ok())
      {
        auto response
          { co_await detail::receive_buffer_with_header<typename Dispatcher::Header>
            ( owned_socket
            )
          };

        if (state->ok())
        {
          asio::co_spawn
            ( executor
            , ConnectionState<Protocol, Dispatcher, HandlerArgs...>::invoke
                ( state
                , std::move (response)
                )
            , asio::bind_executor
              ( strand
              , [state]
                  ( std::exception_ptr rpc_error
                  , detail::ResultHolder result_holder
                  ) noexcept
                {
                  if (state->ok())
                  {
                    if (rpc_error)
                    {
                      state->shutdown();
                    }
                    else
                    {
                      try
                      {
                        asio::write ( state->socket()
                                    , result_holder.archive().buffers()
                                    );
                      }
                      catch (...)
                      {
                        state->shutdown();
                      }
                    }
                  }
                }
              )
            );
        }
      }
    }
    catch (...)
    {
      state->shutdown();
    }
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
          , is_handler_arg... HandlerArgs
          >
    template<typename Executor>
      Provider<Protocol, Dispatcher, HandlerArgs...>::Provider
        ( typename Protocol::endpoint endpoint
        , Executor& executor
        , HandlerArgs... handler_args
        )
          : _state
              { std::make_shared<detail::provider::State<Protocol>>
                  (executor, endpoint)
              }
  {
    _state->start
      ( [ executor = _state->executor()
        , arguments = std::tuple<HandlerArgs...> {handler_args...}
        ] (typename Protocol::socket socket) mutable
        {
          std::apply
            ( [&] (auto&... args)
              {
                asio::co_spawn
                  ( executor
                  , detail::dispatch<Protocol, Dispatcher, HandlerArgs...>
                      (std::move (socket), args...)
                  , asio::detached
                  );
              }
            , arguments
            );
        }
      );
  }

  template< is_protocol Protocol
          , typename Dispatcher
          , is_handler_arg... HandlerArgs
          >
    auto Provider<Protocol, Dispatcher, HandlerArgs...>::local_endpoint
      (
      ) const -> typename Protocol::endpoint
  {
    return _state->local_endpoint();
  }

  template< is_protocol Protocol
          , typename Dispatcher
          , is_handler_arg... HandlerArgs
          >
    auto Provider<Protocol, Dispatcher, HandlerArgs...>::stop_accepting
      (
      ) const -> std::shared_future<void>
  {
    return _state->stop_accepting();
  }

  template< is_protocol Protocol
          , typename Dispatcher
          , is_handler_arg... HandlerArgs
          >
    Provider<Protocol, Dispatcher, HandlerArgs...>::~Provider()
  {
    try
    {
      _state->request_stop();
    }
    catch (...)
    {
      util::FMT::print_noexcept
        ( stderr
        , "rpc::Provider::~Provider: {}\n"
        , std::current_exception()
        );
    }
  }

  template< is_protocol Protocol
          , typename Dispatcher
          , typename Executor
          , is_handler_arg... HandlerArgs
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

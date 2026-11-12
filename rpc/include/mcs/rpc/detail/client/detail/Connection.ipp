// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/co_spawn.hpp>
#include <asio/error.hpp>
#include <asio/post.hpp>
#include <mcs/rpc/detail/receive_buffer_with_header.hpp>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace mcs::rpc::detail::client
{
  template<is_protocol Protocol, typename AdmissionTag>
    Connection<Protocol, AdmissionTag>::Connection
      ( std::shared_ptr<typename Protocol::socket> socket
      )
        : _socket {std::move (socket)}
        , _executor {asio::make_strand (_socket->get_executor())}
  {}

  template<is_protocol Protocol, typename AdmissionTag>
    template<typename MakeSend>
      auto Connection<Protocol, AdmissionTag>::submit
        ( MakeSend make_send
        , detail::Completion completion
        ) -> void
  {
    asio::post
      ( _executor
      , [ self = this->shared_from_this()
        , make_send = std::optional<MakeSend> {std::move (make_send)}
        , completion = std::move (completion)
        ]() mutable
        {
          auto pending {PendingCall {std::move (completion)}};

          auto const fail_pending
            { [&] (auto error)
              {
                make_send.reset();

                pending.complete (error);
              }
            };

          if (self->_terminal_error)
          {
            fail_pending (self->_terminal_error);

            return;
          }

          if constexpr (std::same_as<AdmissionTag, access_policy::Exclusive>)
          {
            if (self->front_send() || self->_completions)
            {
              fail_pending
                ( std::make_exception_ptr
                    ( std::logic_error
                      { "Overlapping calls on an Exclusive client"
                      }
                    )
                );

              return;
            }
          }

          if ( auto const call_id {self->_next_call_id++}
             ; auto operation
                 { [&]() -> std::optional<asio::awaitable<void>>
                   {
                     try
                     {
                       return (*make_send) (self->_socket, call_id);
                     }
                     catch (...)
                     {
                       fail_pending (std::current_exception());

                       return std::nullopt;
                     }
                   }()
                 }
             )
          {
            make_send.reset();

            auto send
              { Send {call_id, std::move (*operation), std::move (pending)}
              };

            if constexpr (std::same_as<AdmissionTag, access_policy::Exclusive>)
            {
              self->_sends.emplace (std::move (send));
            }
            else
            {
              try
              {
                self->_sends.emplace_back (std::move (send));
              }
              catch (...)
              {
                send.operation = {};

                send.call.complete (std::current_exception());

                return;
              }
            }

            self->start_sending();
            self->finish_shutdown();
          }
        }
      );
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::shutdown
      (
      ) -> std::future<void>
  {
    auto promise {std::promise<void>{}};
    auto future {promise.get_future()};

    asio::post
      ( _executor
      , [ self = this->shared_from_this()
        , promise = std::move (promise)
        ]() mutable
        {
          self->_shutdown_waiters.emplace_back (std::move (promise));

          self->close
            ( std::make_exception_ptr
                ( std::system_error {asio::error::operation_aborted}
                )
            );

          self->finish_shutdown();
        }
      );

    return future;
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::front_send() -> Send*
  {
    if constexpr (std::same_as<AdmissionTag, access_policy::Exclusive>)
    {
      return _sends ? std::addressof (*_sends) : nullptr;
    }
    else
    {
      return _sends.empty() ? nullptr : std::addressof (_sends.front());
    }
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::take_send
      ( Send& queued
      ) -> Send
  {
    auto send {std::move (queued)};

    if constexpr (std::same_as<AdmissionTag, access_policy::Exclusive>)
    {
      _sends.reset();
    }
    else
    {
      _sends.pop_front();
    }

    return send;
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::call
      ( CallID call_id
      ) -> PendingCall&
  {
    if constexpr (std::same_as<AdmissionTag, access_policy::Concurrent>)
    {
      if ( auto const completion {_completions.find (call_id)}
         ; completion != _completions.end()
         )
      {
        return completion->second;
      }
    }
    else
    {
      if (_completions && _completions->first == call_id)
      {
        return _completions->second;
      }
    }

    throw std::logic_error {"Response for unknown call_id"};
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::take_call
      ( CallID call_id
      ) -> PendingCall
  {
    auto pending {std::move (call (call_id))};

    if constexpr (std::same_as<AdmissionTag, access_policy::Concurrent>)
    {
      _completions.erase (call_id);
    }
    else
    {
      _completions.reset();
    }

    return pending;
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::start_sending() -> void
  {
    if (_sending || !front_send() || _terminal_error)
    {
      return;
    }

    if constexpr (!std::same_as<AdmissionTag, access_policy::Concurrent>)
    {
      if (_completions)
      {
        return;
      }
    }

    _sending = true;

    auto self {this->shared_from_this()};

    asio::co_spawn
      ( _executor
      , send_requests (self)
      , [self] (std::exception_ptr error)
        {
          self->_sending = false;

          if (error)
          {
            self->close (error);
          }

          self->start_sending();
          self->finish_shutdown();
        }
      );
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::start_receiving() -> void
  {
    if (_receiving || _expected_responses == 0 || _terminal_error)
    {
      return;
    }

    _receiving = true;

    auto self {this->shared_from_this()};

    asio::co_spawn
      ( _executor
      , receive_responses (self)
      , [self] (std::exception_ptr error)
        {
          self->_receiving = false;

          if (error)
          {
            self->close (error);
          }

          self->start_receiving();
          self->finish_shutdown();
        }
      );
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::send_requests
      ( std::shared_ptr<Connection> self
      ) -> asio::awaitable<void>
  {
    while (auto* queued {self->front_send()})
    {
      if (self->_terminal_error)
      {
        co_return;
      }

      if constexpr (!std::same_as<AdmissionTag, access_policy::Concurrent>)
      {
        if (self->_completions)
        {
          co_return;
        }
      }

      auto send {self->take_send (*queued)};

      try
      {
        self->_completions.emplace (send.call_id, std::move (send.call));
      }
      catch (...)
      {
        send.operation = {};

        send.call.complete (std::current_exception());

        throw;
      }

      co_await std::move (send.operation);

      if (self->_terminal_error)
      {
        co_return;
      }

      auto& pending {self->call (send.call_id)};

      pending.sent = true;

      if (pending.response)
      {
        auto response {std::move (*pending.response)};

        self->deliver (std::move (response));
      }

      ++self->_expected_responses;

      if constexpr (std::same_as<AdmissionTag, access_policy::Concurrent>)
      {
        self->start_receiving();
      }
      else
      {
        // A single admitted call can read its response in the same
        // coroutine.
        co_await receive_responses (self);
      }
    }
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::receive_responses
      ( std::shared_ptr<Connection> self
      ) -> asio::awaitable<void>
  {
    while (self->_expected_responses != 0 && !self->_terminal_error)
    {
      auto response
        { co_await receive_buffer_with_header<CallID> (self->_socket)
        };

      if (self->_terminal_error)
      {
        co_return;
      }

      auto const call_id {response.header};
      auto& pending {self->call (call_id)};

      --self->_expected_responses;

      // A response can arrive before its send frame releases borrowed
      // buffers.
      if (pending.sent)
      {
        self->deliver (std::move (response));
      }
      else
      {
        if (pending.response)
        {
          throw std::logic_error {"Duplicate response for call_id"};
        }

        pending.response.emplace (std::move (response));
      }
    }
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::deliver
      ( ReceivedFrame<CallID> response
      ) -> void
  {
    auto const call_id {response.header};
    auto pending {take_call (call_id)};

    pending.complete (std::move (response.buffer));

    start_sending();
  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::close
      ( std::exception_ptr error
      ) -> void
  {
    if (_terminal_error)
    {
      return;
    }

    _terminal_error = error;

    auto close_error {asio::error_code{}};

    _socket->close (close_error);

    if (close_error)
    {
      _socket_close_error = std::make_exception_ptr
        ( std::system_error {close_error, "Closing RPC client connection"}
        );

      _terminal_error = _socket_close_error;
    }

  }

  template<is_protocol Protocol, typename AdmissionTag>
    auto Connection<Protocol, AdmissionTag>::finish_shutdown() -> void
  {
    if (_sending || _receiving)
    {
      return;
    }

    if (_terminal_error)
    {
      // Cancelled I/O must release its buffers before call futures
      // become ready.
      if constexpr (std::same_as<AdmissionTag, access_policy::Concurrent>)
      {
        auto completions {std::exchange (_completions, {})};

        for (auto& [id, pending] : completions)
        {
          pending.complete (_terminal_error);
        }
      }
      else
      {
        if (_completions)
        {
          auto pending {take_call (_completions->first)};

          pending.complete (_terminal_error);
        }
      }

      while (auto* queued {front_send()})
      {
        auto send {take_send (*queued)};
        send.operation = {};
        send.call.complete (_terminal_error);
      }
    }

    for (auto& promise : _shutdown_waiters)
    {
      if (_socket_close_error)
      {
        promise.set_exception (_socket_close_error);
      }
      else
      {
        promise.set_value();
      }
    }

    _shutdown_waiters.clear();
  }
}

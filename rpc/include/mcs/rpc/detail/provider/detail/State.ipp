// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/dispatch.hpp>
#include <asio/error.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/post.hpp>
#include <asio/redirect_error.hpp>
#include <asio/use_awaitable.hpp>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <functional>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/overloaded.hpp>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace mcs::rpc::detail::provider::detail::retry::policy
{
  struct None{};
  struct Prompt{};
  struct Backoff
  {
    static constexpr auto _initial_delay {std::chrono::microseconds {250}};
    static constexpr auto _maximum_delay {std::chrono::microseconds {2500}};
  };
}

namespace mcs::rpc::detail::provider::detail::retry
{
  using Policy = std::variant
    < policy::None
    , policy::Prompt
    , policy::Backoff
    >;
}

namespace mcs::rpc::detail::provider::detail
{
  template<util::ASIO::is_protocol Protocol>
    struct AcceptClients
  {
    template<is_dispatch<Protocol> Dispatch>
      [[nodiscard]] auto operator()
        ( std::shared_ptr<State<Protocol>>
        , Dispatch
        ) const -> asio::awaitable<void>;
  };
}

namespace mcs::rpc::detail::provider
{
  template<util::ASIO::is_protocol Protocol>
    template<typename Executor>
      State<Protocol>::State
        ( Executor& executor
        , typename Protocol::endpoint endpoint
        )
          : _strand {asio::make_strand (executor)}
          , _acceptor {_strand, endpoint}
  {}

  template<util::ASIO::is_protocol Protocol>
    template<detail::is_dispatch<Protocol> Dispatch>
      auto State<Protocol>::start
        ( Dispatch dispatch
        ) -> void
  {
    static constexpr auto accept_clients {detail::AcceptClients<Protocol>{}};

    asio::co_spawn
      ( _strand
      , std::invoke
          ( accept_clients
          , this->shared_from_this()
          , std::move (dispatch)
          )
      , [self = this->shared_from_this()] (std::exception_ptr error)
        {
          self->finish (error);
        }
      );
  }

  template<util::ASIO::is_protocol Protocol>
    auto State<Protocol>::executor
      (
      ) const -> asio::any_io_executor
  {
    return _strand.get_inner_executor();
  }

  template<util::ASIO::is_protocol Protocol>
    auto State<Protocol>::local_endpoint
      (
      ) const -> typename Protocol::endpoint
  {
    return _endpoint;
  }

  template<util::ASIO::is_protocol Protocol>
    auto State<Protocol>::stop_accepting
      (
      ) -> std::shared_future<void>
  {
    request_stop();

    return _stopped_future;
  }

  template<util::ASIO::is_protocol Protocol>
    auto State<Protocol>::request_stop() -> void
  {
    _stop_requested.store (true);

    asio::dispatch
      ( _strand
      , [self = this->shared_from_this()]
        {
          if (!self->_finished)
          {
            self->close();
          }
        }
      );
  }

  template<util::ASIO::is_protocol Protocol>
    auto State<Protocol>::close() -> void
  {
    auto error {asio::error_code{}};

    _acceptor.close (error);
    std::ignore = _retry_timer.cancel();

    if (error && !_close_error)
    {
      _close_error = error;
    }
  }

  template<util::ASIO::is_protocol Protocol>
    auto State<Protocol>::finish
      ( std::exception_ptr error
      ) -> void
  {
    close();

    if (!error && _close_error)
    {
      error = std::make_exception_ptr
        ( std::system_error {_close_error, "Closing RPC listener"}
        );
    }

    _finished = true;

    if (error)
    {
      util::FMT::print_noexcept
        ( stderr
        , "rpc::Provider::accept_clients: {}\n"
        , error
        );

      _stopped.set_exception (error);
    }
    else
    {
      _stopped.set_value();
    }
  }
}

namespace mcs::rpc::detail::provider::detail
{
  template<util::ASIO::is_protocol Protocol>
    [[nodiscard]] auto retry_policy
      ( asio::error_code code
      ) noexcept -> retry::Policy
  {
    if (  code == std::errc::connection_aborted
       || code == std::errc::interrupted
       || code == std::errc::resource_unavailable_try_again
       || code == std::errc::operation_would_block
       )
    {
      return retry::policy::Prompt{};
    }

    if (  code == std::errc::too_many_files_open
       || code == std::errc::too_many_files_open_in_system
       || code == std::errc::no_buffer_space
       || code == std::errc::not_enough_memory
       )
    {
      return retry::policy::Backoff{};
    }

    if constexpr (std::is_same_v<Protocol, asio::ip::tcp>)
    {
      if (  (  code.category() == std::system_category()
            || code.category() == std::generic_category()
            )
         && (  code.value() == ENETDOWN
            || code.value() == EPROTO
            || code.value() == ENOPROTOOPT
            || code.value() == EHOSTDOWN
            || code.value() == ENONET
            || code.value() == EHOSTUNREACH
            || code.value() == EOPNOTSUPP
            || code.value() == ENETUNREACH
            )
         )
      {
        return retry::policy::Prompt{};
      }
    }

    return retry::policy::None{};
  }

  // The accept loop implements the following effective states. They
  // are derived from State members and coroutine-local data rather
  // than represented by an explicit state enum.
  //
  // State          Representation                 Short description
  // -----------------------------------------------------------------
  // Accepting      !stop_requested, !finished     Wait for a client.
  // Prompt retry   recovering, Prompt policy      Yield, then retry.
  // Backoff retry  recovering, Backoff policy     Wait, then retry.
  // Stopping       stop_requested, !finished      Cancel and exit.
  // Finishing      completion handler running     Publish the result.
  // Stopped        finished, fulfilled promise    Normal termination.
  // Failed         finished, exceptional promise  Terminal failure.
  //
  // Accepting:
  // With no waiting client, async_accept suspends the coroutine. A
  // successful accept dispatches the socket and immediately starts
  // the next accept. It also clears recovering and resets retry_delay
  // to Backoff::_initial_delay.
  //
  // Prompt retry:
  // Aborted/interrupted connections and readiness errors retry after
  // yielding to the strand. ASIO normally handles these internally.
  // Linux pending TCP network errors retry likewise, as recommended
  // by accept(2).
  //
  // Backoff retry:
  // Descriptor, buffer, or memory exhaustion start the
  // _retry_timer. Each completed wait doubles retry_delay up to
  // Backoff::_maximum_delay. A successful accept resets the
  // delay. Prompt failures encountered during recovery preserve the
  // current delay.
  //
  // Recovering and diagnostics:
  // Recovering remains true from the first retryable failure until
  // the next successful accept. It controls the "accepting resumed"
  // diagnostic. Failure diagnostics are rate-limited independently of
  // retry delays through next_diagnostic.
  //
  // Stopping:
  // request_stop sets _stop_requested, closes the acceptor, and
  // cancels _retry_timer. Cancellation of either a pending accept or
  // backoff wait is normal shutdown when a stop has been
  // requested. Established connections are owned elsewhere and
  // continue running.
  //
  // Finishing, stopped, and failed:
  // A non-retryable accept error, dispatch error, or unexpected timer
  // error leaves the coroutine exceptionally. The completion handler
  // calls State::finish, which closes remaining resources and
  // fulfills _stopped with either a value or the terminal
  // exception. A close error turns an otherwise normal stop into
  // failure.
  //
  template<util::ASIO::is_protocol Protocol>
    template<is_dispatch<Protocol> Dispatch>
      auto AcceptClients<Protocol>::operator()
        ( std::shared_ptr<State<Protocol>> state
        , Dispatch dispatch
        ) const -> asio::awaitable<void>
  {
    struct Stopped{};

    struct Error
    {
      std::exception_ptr failure;
      retry::Policy policy;
    };

    using AcceptResult = std::variant
      < typename Protocol::socket
      , Stopped
      , Error
      >;

    auto retry_delay {retry::policy::Backoff::_initial_delay};
    auto recovering {false};
    auto next_diagnostic {asio::steady_timer::clock_type::time_point::min()};

    while (!state->_stop_requested.load())
    {
      auto result {AcceptResult {Stopped{}}};

      try
      {
        result = co_await state->_acceptor.async_accept (state->executor());
      }
      catch (std::system_error const& error)
      {
        if (  error.code() != asio::error::operation_aborted
           || !state->_stop_requested.load()
           )
        {
          result = Error
            { .failure = std::current_exception()
            , .policy = retry_policy<Protocol> (error.code())
            };
        }
      }

      auto const retry_accept
        { std::visit
          ( util::overloaded
            { [&] (typename Protocol::socket&& socket) -> retry::Policy
              {
                if (state->_stop_requested.load())
                {
                  return retry::policy::None{};
                }

                if (recovering)
                {
                  util::FMT::print_noexcept
                    ( stderr
                    , "rpc::Provider::accept_clients: accepting resumed\n"
                    );
                }

                recovering = false;
                retry_delay = retry::policy::Backoff::_initial_delay;

                std::invoke (dispatch, std::move (socket));

                return retry::policy::None{};
              }
            , [] (Stopped) noexcept -> retry::Policy
              {
                return retry::policy::None{};
              }
            , [&] (Error const& error) -> retry::Policy
              {
                auto const stopping {state->_stop_requested.load()};
                auto const now {asio::steady_timer::clock_type::now()};
                auto const report_failure
                  { !recovering || now >= next_diagnostic
                  };

                std::visit
                  ( util::overloaded
                    { [&error] (retry::policy::None)
                      {
                        std::rethrow_exception (error.failure);
                      }
                    , [&] (retry::policy::Prompt)
                      {
                        if (stopping)
                        {
                          return;
                        }

                        if (report_failure)
                        {
                          util::FMT::print_noexcept
                            ( stderr
                            , "rpc::Provider::accept_clients: {} "
                              "(retrying without timer delay)\n"
                            , error.failure
                            );
                        }
                      }
                    , [&] (retry::policy::Backoff)
                      {
                        if (stopping)
                        {
                          return;
                        }

                        if (report_failure)
                        {
                          util::FMT::print_noexcept
                            ( stderr
                            , "rpc::Provider::accept_clients: {} "
                              "(retrying in {} us)\n"
                            , error.failure
                            , retry_delay.count()
                            );
                        }

                        state->_retry_timer.expires_after (retry_delay);
                      }
                    }
                  , error.policy
                  );

                if (stopping)
                {
                  return retry::policy::None{};
                }

                if (report_failure)
                {
                  next_diagnostic = now + state->_diagnostic_interval;
                }

                recovering = true;

                return error.policy;
              }
            }
          , std::move (result)
          )
        };

      auto error {asio::error_code{}};
      auto backoff {false};

      if ( auto wait
           { std::visit
             ( util::overloaded
               { [] (retry::policy::None) noexcept -> asio::awaitable<void>
                 {
                   return {};
                 }
               , [&state] (retry::policy::Prompt) -> asio::awaitable<void>
                 {
                   return asio::post (state->_strand, asio::use_awaitable);
                 }
               , [&] (retry::policy::Backoff) -> asio::awaitable<void>
                 {
                   backoff = true;

                   return state->_retry_timer.async_wait
                     ( asio::redirect_error (asio::use_awaitable, error)
                     );
                 }
               }
             , retry_accept
             )
           }
        ; wait.valid()
        )
      {
        co_await std::move (wait);

        if (error)
        {
          if (  error == asio::error::operation_aborted
             && state->_stop_requested.load()
             )
          {
            co_return;
          }

          throw std::system_error {error, "Waiting to retry RPC accept"};
        }

        if (backoff)
        {
          retry_delay = std::min
            ( retry_delay * 2
            , retry::policy::Backoff::_maximum_delay
            );
        }
      }
    }
  }
}

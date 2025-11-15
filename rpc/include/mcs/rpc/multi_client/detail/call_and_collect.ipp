// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <future>
#include <iterator>
#include <list>
#include <mcs/nonstd/scope.hpp>
#include <mcs/rpc/multi_client/Errors.hpp>
#include <mcs/rpc/multi_client/detail/CallID.hpp>
#include <mcs/rpc/multi_client/detail/ClientObserver.hpp>
#include <mcs/rpc/multi_client/detail/Counters.hpp>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace mcs::rpc::multi_client
{
  template< is_command Command
          , typename CommandGenerator
          , typename ClientGenerator
          , typename Collect
          , typename ClientIDs
          >
    requires (  is_result_collect<Collect, ClientIDs, Command>
             && is_command_generator<CommandGenerator, ClientIDs, Command>
             )
    auto call_and_collect
      ( CommandGenerator command
      , ClientGenerator client
      , Collect& collect
      , ClientIDs const& client_ids
      , ParallelCallsLimit parallel_calls_limit
      ) -> void
  {
    auto errors {std::list<std::exception_ptr>{}};

    {
      auto counters {detail::Counters {parallel_calls_limit}};

      struct Call
      {
        decltype (*std::cbegin (client_ids)) _client_id;
        std::future<typename Command::Response> _result;
      };

      auto calls {std::unordered_map<detail::CallID, Call>{}};

      auto const record_error
        { [&] (auto client_id, auto error)
          {
            if constexpr (is_error_collect<Collect, ClientIDs>)
            {
              try
              {
                collect.error (client_id, error);
              }
              catch (...)
              {
                errors.emplace_back (std::current_exception());
              }
            }
            else
            {
              std::ignore = collect;

              errors.emplace_back (error);
            }
          }
        };

    auto const call_completed
      { [&collect, &calls, &record_error] (auto call_id)
        {
          auto const erase_call
            { nonstd::make_scope_exit_that_dies_on_exception
              ( "rpc::multi_call::call_completed"
              , [&]
                {
                  calls.erase (call_id);
                }
              )
            };

          auto& [client_id, result] {calls.at (call_id)};

          try
          {
            if constexpr (std::is_void_v<typename Command::Response>)
            {
              std::ignore = collect;

              result.get();
            }
            else
            {
              collect.result (client_id, result.get());
            }
          }
          catch (...)
          {
            record_error (client_id, std::current_exception());
          }
        }
      };

      auto const calls_completed
        { [&call_completed] (auto const& call_ids)
          {
            std::ranges::for_each (call_ids, call_completed);
          }
        };

      for (auto call_id {detail::CallID{}}; auto const& client_id : client_ids)
      {
        auto const increment_call_id
          {nonstd::make_scope_success ([&]() noexcept { ++call_id; })};

        if constexpr (can_throttle<Collect>)
        {
          if (!collect.start_more_calls())
          {
            break;
          }
        }

        calls_completed (counters.wait_for_slot());

        try
        {
          auto future_result
            { [&]
              {
                if constexpr
                  ( std::is_same_v< decltype (command (client_id))
                                  , std::reference_wrapper<Command const>
                                  >
                  )
                {
                  return std::visit
                    ( [&] (auto&& concrete_client)
                      {
                        return concrete_client
                          . template observe<detail::ClientObserver> (call_id, counters)
                          . template get_future<Command> (command (client_id))
                          ;
                      }
                    , client (client_id)
                    )
                    ;
                }
                else
                {
                  return std::visit
                    ( [&] (auto&& concrete_client)
                      {
                        return concrete_client
                          . template observe<detail::ClientObserver> (call_id, counters)
                          . template async_call<Command> (command (client_id))
                          ;
                      }
                    , client (client_id)
                    )
                    ;
                }
              }
            };

          calls.emplace
            ( std::piecewise_construct
            , std::forward_as_tuple (call_id)
            , std::forward_as_tuple (client_id, future_result())
            );
        }
        catch (...)
        {
          record_error (client_id, std::current_exception());
        }
      }

      while (!calls.empty())
      {
        calls_completed (counters.calls_completed());
      }
    }
    // make sure all clients and counters are destructed -> all
    // communication has been finished

    if (!errors.empty())
    {
      throw Errors {errors};
    }
  }

  template< is_command Command
          , typename CommandGenerator
          , typename Collect
          , typename Clients
          >
    requires (  is_result_collect<Collect, Clients, Command>
             && is_command_generator<CommandGenerator, Clients, Command>
             )
    auto call_and_collect
      ( CommandGenerator command
      , Collect& collect
      , Clients&& clients
      , ParallelCallsLimit parallel_calls_limit
      ) -> void
  {
    return call_and_collect<Command>
      ( std::move (command)
      , [] (auto& client) { return client; }
      , collect
      , std::forward<Clients> (clients)
      , parallel_calls_limit
      );
  }
}

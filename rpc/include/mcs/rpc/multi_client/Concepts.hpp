// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <exception>
#include <iterator>
#include <mcs/rpc/Client.hpp>
#include <utility>

namespace mcs::rpc::multi_client
{
  template<typename Collect, typename ClientIDs, typename Command>
    concept is_result_collect =
       std::is_void_v<typename Command::Response>
    || (  !std::is_void_v<typename Command::Response>
       && requires ( Collect collect
                   , typename Command::Response result
                   , ClientIDs&& client_ids
                   )
          {
            { collect.result (*std::cbegin (client_ids), std::move (result))
            } -> std::convertible_to<void>;
          }
       )
    ;

  template<typename Collect, typename ClientIDs>
    concept is_error_collect =
    requires ( Collect collect
             , std::exception_ptr error
             , ClientIDs&& client_ids
             )
    {
      { collect.error (*std::cbegin (client_ids), error)
      } -> std::convertible_to<void>;
    }
  ;

  template<typename Collect>
    concept can_throttle =
    requires ( Collect collect
             )
    {
      { collect.start_more_calls() } -> std::convertible_to<bool>;
    };

  template<typename Generator, typename Clients, typename Command>
    concept is_command_arguments_generator =
    requires ( Generator& command
             , Clients&& clients
             )
    {
      Command {command (*std::cbegin (clients))};
    };
  template<typename Generator, typename Clients, typename Command>
    concept is_command_reference_generator =
    requires ( Generator& command
             , Clients&& clients
             )
    {
      { command (*std::cbegin (clients)) }
        -> std::convertible_to<std::reference_wrapper<Command const>>;
    };
  template<typename Generator, typename Clients, typename Command>
    concept is_command_generator =
       is_command_arguments_generator<Generator, Clients, Command>
    || is_command_reference_generator<Generator, Clients, Command>
    ;
}

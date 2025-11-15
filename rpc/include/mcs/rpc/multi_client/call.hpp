// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/multi_client/Concepts.hpp>
#include <mcs/rpc/multi_client/ParallelCallsLimit.hpp>
#include <type_traits>

namespace mcs::rpc::multi_client
{
  // Wrappers around call_and_collect that creates and returns a
  // collector.
  //
  template< template<typename> class Collect
          , is_command Command
          , typename CommandGenerator
          , typename ClientGenerator
          , typename ClientIDs
          , typename... CollectArgs
          >
    requires (  std::is_constructible_v<Collect<Command>, CollectArgs...>
             && is_result_collect<Collect<Command>, ClientIDs, Command>
             )
    auto call
      ( CommandGenerator
      , ClientGenerator
      , ClientIDs&&
      , ParallelCallsLimit
      , CollectArgs&&...
      ) -> Collect<Command>
    ;

  template< template<typename> class Collect
          , is_command Command
          , typename CommandGenerator
          , typename Clients
          , typename... CollectArgs
          >
    requires (  std::is_constructible_v<Collect<Command>, CollectArgs...>
             && is_result_collect<Collect<Command>, Clients, Command>
             )
    auto call
      ( CommandGenerator
      , Clients&&
      , ParallelCallsLimit
      , CollectArgs&&...
      ) -> Collect<Command>
    ;
}

#include "detail/call.ipp"

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/multi_client/call_and_collect.hpp>
#include <utility>

namespace mcs::rpc::multi_client
{
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
      ( CommandGenerator command
      , ClientGenerator client
      , ClientIDs&& client_ids
      , ParallelCallsLimit parallel_calls_limit
      , CollectArgs&&... collect_args
      ) -> Collect<Command>
  {
    auto collect
      {Collect<Command> {std::forward<CollectArgs> (collect_args)...}};

    call_and_collect<Command>
      ( std::move (command)
      , std::move (client)
      , collect
      , std::forward<ClientIDs> (client_ids)
      , parallel_calls_limit
      );

    return collect;
  }

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
      ( CommandGenerator command
      , Clients&& clients
      , ParallelCallsLimit parallel_calls_limit
      , CollectArgs&&... collect_args
      ) -> Collect<Command>
  {
    return call<Collect, Command>
      ( std::move (command)
      , [] (auto& client) { return client; }
      , std::forward<Clients> (clients)
      , parallel_calls_limit
      , std::forward<CollectArgs> (collect_args)...
      );
  }
}

// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::rpc
{
  template<typename Client, is_command Command>
    constexpr SyncRemoteFunction<Client, Command>::SyncRemoteFunction
      ( Client& client
      ) noexcept
        : _client {client}
  {}

  template<typename Client, is_command Command>
      constexpr auto SyncRemoteFunction<Client, Command>::operator()
        ( std::reference_wrapper<Command const> command
        ) -> typename Command::Response
  {
    return _client (command);
  }

  template<typename Client, is_command Command>
      constexpr auto SyncRemoteFunction<Client, Command>::operator()
        ( Command&& command
        ) -> typename Command::Response
  {
    return _client (std::forward<Command> (command));
  }

  template<typename Client, is_command Command>
    template<typename... CommandArgs>
      constexpr auto SyncRemoteFunction<Client, Command>::operator()
        ( CommandArgs&&... command_args
        ) -> typename Command::Response
  {
    return _client.template call<Command>
      (std::forward<CommandArgs> (command_args)...);
  }

  template<is_command Command, typename Client>
    constexpr auto make_sync_remote_function
      ( Client& client
      ) noexcept
  {
    return SyncRemoteFunction<Client, Command> {client};
  }
}

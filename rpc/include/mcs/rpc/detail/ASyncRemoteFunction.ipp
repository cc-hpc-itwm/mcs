// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::rpc
{
  template<typename Client, is_command Command>
    constexpr ASyncRemoteFunction<Client, Command>::ASyncRemoteFunction
      ( Client& client
      ) noexcept
        : _client {client}
  {}

  template<typename Client, is_command Command>
      constexpr auto ASyncRemoteFunction<Client, Command>::operator()
        ( std::reference_wrapper<Command const> command
        ) -> std::future<typename Command::Response>
  {
    return _client.get_future (command);
  }

  template<typename Client, is_command Command>
      constexpr auto ASyncRemoteFunction<Client, Command>::operator()
        ( Command&& command
        ) -> std::future<typename Command::Response>
  {
    return _client.get_future (std::forward<Command> (command));
  }

  template<typename Client, is_command Command>
    template<typename... CommandArgs>
      constexpr auto ASyncRemoteFunction<Client, Command>::operator()
        ( CommandArgs&&... command_args
        ) -> std::future<typename Command::Response>
  {
    return _client.template async_call<Command>
      (std::forward<CommandArgs> (command_args)...);
  }

  template<is_command Command, typename Client>
    constexpr auto make_async_remote_function
      ( Client& client
      ) noexcept
  {
    return ASyncRemoteFunction<Client, Command> {client};
  }
}

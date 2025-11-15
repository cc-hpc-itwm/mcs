// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::rpc::detail::command_holder
{
  template<typename Command>
    template<typename... CommandArgs>
      constexpr Owning<Command>::Owning
        ( CommandArgs&&... command_args
        ) noexcept (std::is_nothrow_constructible_v<Command, CommandArgs...>)
          : _command {std::forward<CommandArgs> (command_args)...}
  {}

  template<typename Command>
    constexpr auto Owning<Command>::ref() const noexcept -> Command const&
  {
    return _command;
  }
}

// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <type_traits>

namespace mcs::rpc::detail::command_holder
{
  template<typename Command>
    struct Owning
  {
    template<typename... CommandArgs>
      constexpr explicit Owning
        ( CommandArgs&&...
        ) noexcept (std::is_nothrow_constructible_v<Command, CommandArgs...>)
      ;

    constexpr auto ref() const noexcept -> Command const&;

  private:
    Command _command;
  };
}

#include "detail/Owning.ipp"

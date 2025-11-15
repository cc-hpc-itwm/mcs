// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>

namespace mcs::rpc::detail::command_holder
{
  template<typename Command>
    struct NonOwning
  {
    constexpr explicit NonOwning
      ( std::reference_wrapper<Command const>
      ) noexcept;

    constexpr auto ref() const noexcept -> Command const&;

  private:
    std::reference_wrapper<Command const> _command;
  };
}

#include "detail/NonOwning.ipp"

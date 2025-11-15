// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstdint>
#include <fmt/format.h>

namespace mcs::rpc::detail
{
  struct CommandIndex
  {
    constexpr CommandIndex() noexcept = default;

    constexpr auto operator++() noexcept -> CommandIndex&;
    constexpr auto operator--() noexcept -> CommandIndex&;

    constexpr auto operator<=> (CommandIndex const&) const noexcept = default;

  private:
    template<typename, typename, typename> friend struct fmt::formatter;

    std::size_t _value {0};
  };


  template<typename T, typename Head, typename... Ts>
    constexpr auto command_index() noexcept -> CommandIndex;
}

#include "detail/CommandIndex.ipp"

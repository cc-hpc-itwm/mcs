// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstdint>
#include <fmt/format.h>

namespace mcs::rpc::detail
{
  struct CallID
  {
    constexpr CallID() noexcept = default;

    constexpr auto operator++(int) noexcept -> CallID;
    constexpr auto operator++() noexcept -> CallID&;

    constexpr auto operator<=> (CallID const&) const noexcept = default;

  private:
    friend struct fmt::formatter<CallID>;

    std::size_t _value {0};
  };
}

#include "detail/CallID.ipp"

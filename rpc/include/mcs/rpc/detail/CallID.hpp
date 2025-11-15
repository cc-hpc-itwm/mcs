// Copyright (C) 2022-2025 Fraunhofer ITWM
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
    template<typename, typename, typename> friend struct fmt::formatter;

    std::size_t _value {0};
  };
}

#include "detail/CallID.ipp"

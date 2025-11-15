// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util
{
  template<typename Char>
    [[nodiscard]] constexpr auto isdigit (Char) noexcept -> bool;
}

#include <cctype>

namespace mcs::util
{
  template<typename Char>
    constexpr auto isdigit (Char c) noexcept -> bool
  {
    return std::isdigit (static_cast<unsigned char> (c));
  }
}

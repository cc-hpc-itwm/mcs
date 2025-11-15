// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util
{
  template<typename Char>
    [[nodiscard]] constexpr auto isspace (Char) noexcept -> bool;
}

#include <cctype>

namespace mcs::util
{
  template<typename Char>
    constexpr auto isspace (Char c) noexcept -> bool
  {
    return std::isspace (static_cast<unsigned char> (c));
  }
}

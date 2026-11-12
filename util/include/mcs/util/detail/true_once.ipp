// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::util
{
  constexpr TrueOnce::operator bool() const noexcept
  {
    return std::exchange (_value, false);
  }
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstddef>
#include <latch>
#include <mcs/core/memory/Size.hpp>

namespace
{
  struct Progress
  {
    explicit Progress (mcs::core::memory::Size);
    auto bytes_transferred (mcs::core::memory::Size);
    auto wait_done();

  private:
    /* constexpr cxx23 */ auto ptrdiff (mcs::core::memory::Size) -> std::ptrdiff_t;
    std::latch _bytes_to_transfer;
  };
}

#include "detail/Progress.ipp"

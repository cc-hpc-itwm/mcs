// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/cast.hpp>

namespace
{
  Progress::Progress (mcs::core::memory::Size size)
    : _bytes_to_transfer {ptrdiff (size)}
  {}

  auto Progress::bytes_transferred (mcs::core::memory::Size size)
  {
    _bytes_to_transfer.count_down (ptrdiff (size));
  }

  auto Progress::wait_done()
  {
    _bytes_to_transfer.wait();
  }

  /* constexpr cxx23 */ auto Progress::ptrdiff
    ( mcs::core::memory::Size size
    ) -> std::ptrdiff_t
  {
    return mcs::util::cast<std::ptrdiff_t> (size_cast<std::size_t> (size));
  }
}

// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>

namespace mcs::util::syscall
{
  auto munmap (void* addr, size_t length) -> void;

  // The opposite to mmap_with_length_zero_allowed:
  //
  // If length == 0, then Requires: addr == nullptr and does nothing.
  // If length != 0, then calls munmap (addr, length).
  //
  auto munmap_with_length_zero_allowed (void* addr, size_t length) -> void;
}

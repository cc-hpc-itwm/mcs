// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <sys/mman.h>
#include <sys/types.h>

namespace mcs::util::syscall
{
  auto mmap (void* addr, size_t length, int prot, int flags, int fd, off_t offset) -> void*;

  // mmap does not accept length = 0 as parameter. This function
  // accepts length = 0 and does not map anything in this case. This
  // is useful if clients want to use mmap/munmap pairs with a user
  // given size.
  //
  // Returns: if length == 0, then nullptr
  //          if length != 0, then mmap (addr, length, prot, flags, fd, offset)
  //
  auto mmap_with_length_zero_allowed
    ( void* addr
    , size_t length
    , int prot
    , int flags
    , int fd
    , off_t offset
    ) -> void*
    ;
}

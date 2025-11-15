// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace mcs::util::syscall
{
  auto shm_open (const char* name, int oflag, mode_t mode) -> int;
}

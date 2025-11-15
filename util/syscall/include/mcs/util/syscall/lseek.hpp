// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <sys/types.h>

namespace mcs::util::syscall
{
  auto lseek (int fd, off_t, int whence) -> off_t;
}

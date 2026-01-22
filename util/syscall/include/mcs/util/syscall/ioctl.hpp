// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <sys/ioctl.h>

namespace mcs::util::syscall
{
  auto ioctl (int fd, unsigned long request, void* argn) -> int;
}

// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <unistd.h>

namespace mcs::util::syscall
{
  auto pread (int fd, void* buf, size_t nbyte, off_t offset) -> ssize_t;
}

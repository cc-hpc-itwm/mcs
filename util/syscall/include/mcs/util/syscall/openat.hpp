// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <sys/types.h>

namespace mcs::util::syscall
{
  [[nodiscard]] auto openat (int, char const*, int, mode_t = 0) -> int;
}

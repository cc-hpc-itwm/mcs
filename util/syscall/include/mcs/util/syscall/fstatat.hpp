// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <sys/stat.h>

namespace mcs::util::syscall
{
  [[nodiscard]] auto fstatat (int, char const*, int) -> struct stat;
}

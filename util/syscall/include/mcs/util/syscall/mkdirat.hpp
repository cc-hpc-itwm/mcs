// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <sys/types.h>

namespace mcs::util::syscall
{
  auto mkdirat (int, char const*, mode_t) -> void;
}

// Copyright (C) 2024-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <sys/resource.h>

namespace mcs::testing::syscall
{
  auto getrlimit (int) -> rlimit;
}

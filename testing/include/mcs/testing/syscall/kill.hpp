// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <csignal>

namespace mcs::testing::syscall
{
  auto kill (pid_t, int) -> void;
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <sys/types.h>

namespace mcs::testing::syscall
{
  auto waitpid_retrying_on_interrupt (pid_t, int*, int) -> pid_t;
}

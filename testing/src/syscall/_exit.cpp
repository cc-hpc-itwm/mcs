// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/syscall/_exit.hpp>
#include <unistd.h>

namespace mcs::testing::syscall
{
  [[noreturn]] auto _exit (int status) -> void
  {
    ::_exit (status);
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <unistd.h>

namespace mcs::util::syscall
{
  auto getpid() -> pid_t
  {
    return never_fails (::getpid());
  }
}

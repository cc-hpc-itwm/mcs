// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/geteuid.hpp>

namespace mcs::util::syscall
{
  auto geteuid() -> uid_t
  {
    return never_fails (::geteuid());
  }
}

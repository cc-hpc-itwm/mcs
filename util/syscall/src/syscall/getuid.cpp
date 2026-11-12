// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/getuid.hpp>

namespace mcs::util::syscall
{
  auto getuid() -> uid_t
  {
    return never_fails (::getuid());
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/getegid.hpp>

namespace mcs::util::syscall
{
  auto getegid() -> gid_t
  {
    return never_fails (::getegid());
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/setsid.hpp>
#include <unistd.h>

namespace mcs::util::syscall
{
  auto setsid() -> pid_t
  try
  {
    return negative_one_throws_with_errno<pid_t> (::setsid());
  }
  catch (...)
  {
    std::throw_with_nested (Error {"syscall::setsid()"});
  }
}

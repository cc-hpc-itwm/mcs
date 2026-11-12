// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/testing/syscall/waitpid.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <sys/wait.h>

namespace mcs::testing::syscall
{
  auto waitpid (pid_t pid, int* status, int options) -> pid_t
  try
  {
    return util::syscall::negative_one_throws_with_errno<pid_t>
      ( ::waitpid (pid, status, options)
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( mcs::util::syscall::Error
        { fmt::format
          ( "syscall::waitpid (pid = {}, status = {}, options = {})"
          , pid
          , util::cast<void*> (status)
          , options
          )
        }
      );
  }
}

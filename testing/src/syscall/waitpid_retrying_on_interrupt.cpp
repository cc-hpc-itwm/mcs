// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cerrno>
#include <exception>
#include <fmt/format.h>
#include <mcs/testing/syscall/waitpid_retrying_on_interrupt.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <sys/wait.h>

namespace mcs::testing::syscall
{
  auto waitpid_retrying_on_interrupt
    ( pid_t pid
    , int* status
    , int options
    ) -> pid_t
  try
  {
    auto result {pid_t {-1}};

    do
    {
      result = ::waitpid (pid, status, options);
    }
    while (result == -1 && errno == EINTR);

    return util::syscall::negative_one_throws_with_errno<pid_t> (result);
  }
  catch (...)
  {
    std::throw_with_nested
      ( util::syscall::Error
        { fmt::format
          ( "syscall::waitpid_retrying_on_interrupt"
            " (pid = {}, status = {}, options = {})"
          , pid
          , util::cast<void*> (status)
          , options
          )
        }
      );
  }
}

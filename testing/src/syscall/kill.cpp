// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/testing/syscall/kill.hpp>
#include <mcs/util/syscall/Error.hpp>

namespace mcs::testing::syscall
{
  auto kill (pid_t pid, int signal) -> void
  try
  {
    return util::syscall::negative_one_throws_with_errno<void>
      ( ::kill (pid, signal)
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( mcs::util::syscall::Error
        { fmt::format ("syscall::kill (pid = {}, signal = {})", pid, signal)
        }
      );
  }
}

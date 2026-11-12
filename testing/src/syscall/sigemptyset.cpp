// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/testing/syscall/sigemptyset.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/Error.hpp>

namespace mcs::testing::syscall
{
  auto sigemptyset (sigset_t* set) -> void
  try
  {
    return util::syscall::negative_one_throws_with_errno<void>
      ( ::sigemptyset (set)
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( mcs::util::syscall::Error
        { fmt::format
          ( "syscall::sigemptyset (set = {})", util::cast<void*> (set)
          )
        }
      );
  }
}

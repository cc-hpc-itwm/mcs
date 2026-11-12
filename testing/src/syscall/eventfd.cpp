// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/testing/syscall/eventfd.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <sys/eventfd.h>

namespace mcs::testing::syscall
{
  auto eventfd (unsigned int initval, int flags) -> int
  try
  {
    return util::syscall::negative_one_throws_with_errno<int>
      ( ::eventfd (initval, flags)
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( mcs::util::syscall::Error
        { fmt::format
          ( "syscall::eventfd (initval = {}, flags = {})"
          , initval
          , flags
          )
        }
      );
  }
}

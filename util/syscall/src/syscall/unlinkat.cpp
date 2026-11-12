// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/unlinkat.hpp>
#include <unistd.h>

namespace mcs::util::syscall
{
  auto unlinkat (int directory, char const* path, int flags) -> void
  try
  {
    return negative_one_throws_with_errno<void>
      ( ::unlinkat (directory, path, flags)
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::unlinkat (directory = {}, path = '{}', flags = {})"
          , directory
          , path
          , flags
          )
        }
      );
  }
}

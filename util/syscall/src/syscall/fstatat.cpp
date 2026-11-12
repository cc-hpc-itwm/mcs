// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/fstatat.hpp>
#include <memory>

namespace mcs::util::syscall
{
  auto fstatat (int directory, char const* path, int flags) -> struct stat
  try
  {
    struct stat status {};

    negative_one_throws_with_errno<void>
      ( ::fstatat (directory, path, std::addressof (status), flags)
      );

    return status;
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::fstatat (directory = {}, path = '{}', flags = {})"
          , directory
          , path
          , flags
          )
        }
      );
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/mkdirat.hpp>
#include <sys/stat.h>

namespace mcs::util::syscall
{
  auto mkdirat (int directory, char const* path, mode_t mode) -> void
  try
  {
    return negative_one_throws_with_errno<void>
      ( ::mkdirat (directory, path, mode)
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::mkdirat (directory = {}, path = '{}', mode = {})"
          , directory
          , path
          , mode
          )
        }
      );
  }
}

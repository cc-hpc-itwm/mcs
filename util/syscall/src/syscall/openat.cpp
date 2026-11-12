// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fcntl.h>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/openat.hpp>

namespace mcs::util::syscall
{
  auto openat
    ( int directory
    , char const* path
    , int flags
    , mode_t mode
    ) -> int
  try
  {
    return negative_one_throws_with_errno<int>
      ( ::openat (directory, path, flags, mode)
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::openat"
            " (directory = {}, path = '{}', flags = {}, mode = {})"
          , directory
          , path
          , flags
          , mode
          )
        }
      );
  }
}

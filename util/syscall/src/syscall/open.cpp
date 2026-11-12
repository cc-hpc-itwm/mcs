// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fcntl.h>
#include <fmt/format.h>
#include <mcs/util/FMT/STD/FILESYSTEM/Path.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/open.hpp>

namespace mcs::util::syscall
{
  auto open (char const* path, int flags, mode_t mode) -> int
  try
  {
    return negative_one_throws_with_errno<int> (::open (path, flags, mode));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::open (path = {}, flags = {}, mode = {})"
          , FMT::STD::FILESYSTEM::Path {path}
          , flags
          , mode
          )
        }
      );
  }
}

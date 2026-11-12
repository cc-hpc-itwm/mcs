// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/shm_open.hpp>

namespace mcs::util::syscall
{
  auto shm_open (const char* name, int oflag, mode_t mode) -> int
  try
  {
    return negative_one_throws_with_errno<int> (::shm_open (name, oflag, mode));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::shm_open (name = '{}', oflag = {}, mode = {})"
                      , name
                      , oflag
                      , mode
                      )
        }
      );
  }
}

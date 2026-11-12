// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/renameat2.hpp>
#include <sys/syscall.h>
#include <unistd.h>

namespace mcs::util::syscall
{
  auto renameat2
    ( int source_directory
    , char const* source
    , int destination_directory
    , char const* destination
    , unsigned int flags
    ) -> void
  try
  {
    return negative_one_throws_with_errno<void>
      ( ::syscall
        ( SYS_renameat2
        , source_directory
        , source
        , destination_directory
        , destination
        , flags
        )
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::renameat2 (source_directory = {}, source = '{}',"
            " destination_directory = {}, destination = '{}', flags = {})"
          , source_directory
          , source
          , destination_directory
          , destination
          , flags
          )
        }
      );
  }
}

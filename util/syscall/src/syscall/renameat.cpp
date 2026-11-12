// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdio>
#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/renameat.hpp>

namespace mcs::util::syscall
{
  auto renameat
    ( int source_directory
    , char const* source
    , int destination_directory
    , char const* destination
    ) -> void
  try
  {
    return negative_one_throws_with_errno<void>
      ( ::renameat ( source_directory
                   , source
                   , destination_directory
                   , destination
                   )
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::renameat (source_directory = {}, source = '{}',"
            " destination_directory = {}, destination = '{}')"
          , source_directory
          , source
          , destination_directory
          , destination
          )
        }
      );
  }
}

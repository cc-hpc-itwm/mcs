// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/realloc.hpp>

namespace mcs::util::syscall
{
  auto realloc (void* pointer, size_t size) -> void*
  try
  {
    return nullptr_fails_with_errno (::realloc (pointer, size));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::realloc (pointer = {}, size = {})"
                      , pointer
                      , size
                      )
        }
      );
  }
}

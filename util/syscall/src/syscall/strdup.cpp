// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/strdup.hpp>

namespace mcs::util::syscall
{
  auto strdup (char const* s) -> char*
  try
  {
    return nullptr_fails_with_errno (::strdup (s));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::strdup (s = {})"
                      , s
                      )
        }
      );
  }
}

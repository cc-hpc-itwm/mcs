// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/dlopen.hpp>

namespace mcs::util::syscall
{
  auto dlopen (char const* filename, int flag) -> void*
  try
  {
    return detail::CheckDLError {"dlopen"} (::dlopen (filename, flag));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::dlopen (filename = '{}', flag = {})"
                      , filename
                      , flag
                      )
        }
      );
  }
}

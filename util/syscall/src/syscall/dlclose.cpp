// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/dlclose.hpp>

namespace mcs::util::syscall
{
  auto dlclose (void* handle) -> int
  try
  {
    return detail::CheckDLError {"dlclose"} (::dlclose (handle));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::dlclose (handle = {})", handle)
        }
      );
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/dlsym.hpp>

namespace mcs::util::syscall
{
  auto dlsym (void* handle, char const* symbol) -> void*
  try
  {
    return detail::CheckDLError {"dlsym"} (::dlsym (handle, symbol));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::dlsym (handle = {}, symbol = '{}')"
                      , handle
                      , symbol
                      )
        }
      );
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cerrno>
#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/sysconf.hpp>

namespace mcs::util::syscall
{
  auto sysconf (int name) -> long
  try
  {
    errno = 0;
    auto const value {::sysconf (name)};
    if (value == -1 && errno != 0)
    {
      throw syscall_error (errno);
    }
    return value;
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::sysconf (name = {})", name)
        }
      );
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/raise.hpp>

namespace mcs::util::syscall
{
  auto raise (int sig) -> void
  try
  {
    auto const non_null_is_failure
      { [] (int rc) -> void
        {
          if (std::cmp_not_equal (rc, 0))
          {
            throw syscall_error (rc);
          }
        }
      };

    return non_null_is_failure (::raise (sig));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error {fmt::format ( "syscall::raise (sig = {})", sig)}
      );
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/fstat.hpp>
#include <memory>

namespace mcs::util::syscall
{
  auto fstat (int descriptor) -> struct stat
  try
  {
    struct stat status{};

    negative_one_throws_with_errno<void>
      ( ::fstat (descriptor, std::addressof (status))
      );

    return status;
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error {fmt::format ("syscall::fstat (descriptor = {})", descriptor)}
      );
  }
}

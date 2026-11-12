// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/dup.hpp>
#include <unistd.h>

namespace mcs::util::syscall
{
  auto dup (int oldfd) -> int
  try
  {
    return negative_one_throws_with_errno<int> (::dup (oldfd));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error {fmt::format ("syscall::dup (oldfd = {})", oldfd)}
      );
  }
}

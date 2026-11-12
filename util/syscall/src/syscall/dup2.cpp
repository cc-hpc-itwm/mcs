// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/dup2.hpp>
#include <unistd.h>

namespace mcs::util::syscall
{
  auto dup2 (int oldfd, int newfd) -> int
  try
  {
    return negative_one_throws_with_errno<int> (::dup2 (oldfd, newfd));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::dup2 (oldfd = {}, newfd = {})", oldfd, newfd)
        }
      );
  }
}

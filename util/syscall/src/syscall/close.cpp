// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/close.hpp>
#include <unistd.h>

namespace mcs::util::syscall
{
  auto close (int fd) -> void
  try
  {
    return negative_one_throws_with_errno<void> (::close (fd));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::close (fd = {})", fd)
        }
      );
  }
}

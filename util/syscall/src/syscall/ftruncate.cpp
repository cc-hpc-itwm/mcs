// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/ftruncate.hpp>
#include <unistd.h>

namespace mcs::util::syscall
{
  auto ftruncate (int fd, off_t length) -> void
  try
  {
    return negative_one_throws_with_errno<void> (::ftruncate (fd, length));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::ftruncate (fd = {}, length = {})", fd, length)
        }
      );
  }
}

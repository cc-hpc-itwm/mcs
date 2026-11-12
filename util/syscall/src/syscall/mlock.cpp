// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/mlock.hpp>
#include <sys/mman.h>

namespace mcs::util::syscall
{
  auto mlock (void const* addr, size_t length) -> void
  try
  {
    return negative_one_throws_with_errno<void> (::mlock (addr, length));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::mlock (addr = {}, length = {})", addr, length)
        }
      );
  }
}

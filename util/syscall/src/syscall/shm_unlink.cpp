// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/shm_unlink.hpp>

namespace mcs::util::syscall
{
  auto shm_unlink (const char* name) -> void
  try
  {
    return negative_one_throws_with_errno<void> (::shm_unlink (name));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::shm_unlink (name = '{}')", name)
        }
      );
  }
}

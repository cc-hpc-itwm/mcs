// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/munmap.hpp>
#include <sys/mman.h>
#include <utility>

namespace mcs::util::syscall
{
  auto munmap (void* addr, size_t length) -> void
  try
  {
    return negative_one_throws_with_errno<void> (::munmap (addr, length));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::munmap (addr = {}, length = {})"
                      , addr
                      , length
                      )
        }
      );
  }

  auto munmap_with_length_zero_allowed (void* addr, size_t length) -> void
  try
  {
    if (std::cmp_equal (length, 0))
    {
      if (addr != nullptr)
      {
        throw mcs::Error {"addr must be equal to nullptr"};
      }

      return;
    }

    return munmap (addr, length);
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::munmap_with_length_zero_allowed (addr = {}, length = {})"
          , addr
          , length
          )
        }
      );
  }
}

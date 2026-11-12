// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/mmap.hpp>
#include <mcs/util/syscall/sysconf.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::util::syscall
{
  auto mmap (void* addr, size_t length, int prot, int flags, int fd, off_t offset) -> void*
  try
  {
    auto const page_size {util::syscall::sysconf (_SC_PAGE_SIZE)};

    if (offset % page_size != 0)
    {
      // \todo specific exception
      throw std::invalid_argument
        { fmt::format
          ( "mmap: offset must be a multiple of the page_size '{}'"
            " and offset {} = {} * {} + {}"
          , page_size
          , offset
          , offset / page_size
          , page_size
          , offset % page_size
          )
        };
    }

    auto const MAP_FAILED_fails_with_errno
      { [] (void* rc) -> void*
        {
          if (rc == MAP_FAILED)
          {
            throw syscall_error (errno);
          }

          return rc;
        }
      };

    return MAP_FAILED_fails_with_errno
      (::mmap (addr, length, prot, flags, fd, offset));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::mmap"
            " (addr = {}"
            ", length = {}"
            ", prot = {}"
            ", flags = {}"
            ", fd = {}"
            ", offset = {}"
            ")"
          , addr
          , length
          , prot
          , flags
          , fd
          , offset
          )
        }
      );
  }

  auto mmap_with_length_zero_allowed
    ( void* addr
    , size_t length
    , int prot
    , int flags
    , int fd
    , off_t offset
    ) -> void*
  try
  {
    if (std::cmp_equal (length, 0))
    {
      return nullptr;
    }

    return mmap (addr, length, prot, flags, fd, offset);
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::mmap_with_length_zero_allowed"
            " (addr = {}"
            ", length = {}"
            ", prot = {}"
            ", flags = {}"
            ", fd = {}"
            ", offset = {}"
            ")"
          , addr
          , length
          , prot
          , flags
          , fd
          , offset
          )
        }
      );
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/pwrite.hpp>

namespace mcs::util::syscall
{
  auto pwrite (int fd, void const* buf, size_t nbyte, off_t offset) -> ssize_t
  try
  {
    return negative_one_throws_with_errno<ssize_t>
      (::pwrite (fd, buf, nbyte, offset));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::pwrite (fd = {}, buf = {}, nbyte = {}, offset = {})"
          , fd
          , buf
          , nbyte
          , offset
          )
        }
      );
  }
}

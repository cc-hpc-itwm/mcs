// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/write.hpp>

namespace mcs::util::syscall
{
  auto write (int fd, void const* buf, size_t nbyte) -> ssize_t
  try
  {
    return negative_one_throws_with_errno<ssize_t> (::write (fd, buf, nbyte));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::write (fd = {}, buf = {}, nbyte = {})"
                      , fd
                      , buf
                      , nbyte
                      )
        }
      );
  }
}

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/read.hpp>

namespace mcs::util::syscall
{
  auto read (int fd, void* buf, size_t nbyte) -> ssize_t
  try
  {
    return negative_one_throws_with_errno<ssize_t> (::read (fd, buf, nbyte));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::read (fd = {}, buf = {}, nbyte = {})"
                      , fd
                      , buf
                      , nbyte
                      )
        }
      );
  }
}

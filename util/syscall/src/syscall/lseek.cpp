// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/lseek.hpp>
#include <unistd.h>

namespace mcs::util::syscall
{
  auto lseek (int fd, off_t offset, int whence) -> off_t
  try
  {
    return negative_one_throws_with_errno<off_t> (::lseek (fd, offset, whence));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::lseek (fd = {}, offset = {}, whence = {})"
                      , fd
                      , offset
                      , whence
                      )
        }
      );
  }
}

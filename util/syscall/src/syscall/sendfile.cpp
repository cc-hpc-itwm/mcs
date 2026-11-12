// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/sendfile.hpp>

namespace mcs::util::syscall
{
  auto sendfile
    ( int out_fd
    , int in_fd
    , off_t* offset
    , size_t count
    ) -> ssize_t
  try
  {
    return negative_one_throws_with_errno<ssize_t>
      ( ::sendfile (out_fd, in_fd, offset, count)
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::sendfile"
            " (out_fd = {}"
            ", in_fd = {}"
            ", offset = {}@{}"
            ", count = {}"
            ")"
          , out_fd
          , in_fd
          , offset ? *offset : off_t {-1}
          , cast<void*> (offset)
          , count
          )
        }
      );
  }
}

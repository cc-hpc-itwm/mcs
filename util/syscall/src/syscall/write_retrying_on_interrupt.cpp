// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/write_retrying_on_interrupt.hpp>

namespace mcs::util::syscall
{
  auto write_retrying_on_interrupt
    ( int fd
    , void const* buf
    , size_t nbyte
    ) -> ssize_t
  try
  {
    auto result {ssize_t {-1}};

    do
    {
      result = ::write (fd, buf, nbyte);
    }
    while (result == -1 && errno == EINTR);

    return negative_one_throws_with_errno<ssize_t> (result);
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::write_retrying_on_interrupt"
            " (fd = {}, buf = {}, nbyte = {})"
          , fd
          , buf
          , nbyte
          )
        }
      );
  }
}

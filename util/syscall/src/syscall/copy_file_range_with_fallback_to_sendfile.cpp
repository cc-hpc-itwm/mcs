// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cerrno>
#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/copy_file_range_with_fallback_to_sendfile.hpp>
#include <sys/sendfile.h>

namespace mcs::util::syscall
{
  auto copy_file_range_with_fallback_to_sendfile
    ( int fd_in
    , int fd_out
    , size_t len
    ) -> ssize_t
  try
  {
    auto const r
      {::copy_file_range (fd_in, nullptr, fd_out, nullptr, len, 0)};

    if (r == -1 && errno == EXDEV)
    {
      return ::sendfile (fd_out, fd_in, nullptr, len);
    }

    return negative_one_throws_with_errno<ssize_t> (r);
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::copy_file_range_with_fallback_to_sendfile"
            " (fd_in = {}"
            ", fd_out = {}"
            ", len = {}"
            ")"
          , fd_in
          , fd_out
          , len
          )
        }
      );
  }
}

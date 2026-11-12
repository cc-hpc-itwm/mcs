// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/copy_file_range.hpp>

namespace mcs::util::syscall
{
  auto copy_file_range
    ( int fd_in
    , off_t* off_in
    , int fd_out
    , off_t* off_out
    , size_t len
    , unsigned int flags
    ) -> ssize_t
  try
  {
    return negative_one_throws_with_errno<ssize_t>
      (::copy_file_range (fd_in, off_in, fd_out, off_out, len, flags));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::copy_file_range"
            " (fd_in = {}"
            ", off_in = {}@{}"
            ", fd_out = {}"
            ", off_out = {}@{}"
            ", len = {}"
            ", flags = {}"
            ")"
          , fd_in
          , off_in ? *off_in : off_t {-1}
          , cast<void*> (off_in)
          , fd_out
          , off_out ? *off_out : off_t {-1}
          , cast<void*> (off_out)
          , len
          , flags
          )
        }
      );
  }
}

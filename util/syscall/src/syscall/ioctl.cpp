// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/ioctl.hpp>

namespace mcs::util::syscall
{
  auto ioctl (int fd, unsigned long request, void* arg) -> int
  try
  {
    // \note on alpine linux the type for the request is "int"
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wconversion"
    return negative_one_throws_with_errno<int> (::ioctl (fd, request, arg));
    #pragma GCC diagnostic pop
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::ioctl (fd = {}, request = {}, arg = {})"
                      , fd
                      , request
                      , arg
                      )
        }
      );
  }
}

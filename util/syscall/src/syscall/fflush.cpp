// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/fflush.hpp>

namespace mcs::util::syscall
{
  auto fflush (FILE* stream) -> void
  try
  {
    if (::fflush (stream) == EOF)
    {
      throw syscall_error (errno);
    }
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::fflush (stream = {})", cast<void*> (stream))
        }
      );
  }
}

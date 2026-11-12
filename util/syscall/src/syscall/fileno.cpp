// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/fileno.hpp>

namespace mcs::util::syscall
{
  auto fileno (FILE* stream) -> int
  try
  {
    return negative_one_throws_with_errno<int> (::fileno (stream));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::fileno (stream = {})", cast<void*> (stream))
        }
      );
  }
}

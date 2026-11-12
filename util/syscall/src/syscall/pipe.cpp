// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/pipe.hpp>
#include <unistd.h>

namespace mcs::util::syscall
{
  auto pipe() -> Pipe
  try
  {
    auto descriptors {std::array<int, 2>{}};

    negative_one_throws_with_errno<void> (::pipe (descriptors.data()));

    return Pipe {descriptors};
  }
  catch (...)
  {
    std::throw_with_nested (Error {"syscall::pipe()"});
  }
}

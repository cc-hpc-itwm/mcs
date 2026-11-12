// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/statfs.hpp>
#include <memory>

namespace mcs::util::syscall
{
  auto statfs (char const* path) -> struct statfs
  try
  {
    struct statfs stat {};
    negative_one_throws_with_errno<void>
      (::statfs (path, std::addressof (stat)));
    return stat;
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::statfs (path = '{}')", path)
        }
      );
  }
}

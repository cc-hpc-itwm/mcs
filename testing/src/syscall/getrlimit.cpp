// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/testing/syscall/getrlimit.hpp>
#include <mcs/util/syscall/Error.hpp>

namespace mcs::testing::syscall
{
  auto getrlimit (int resource) -> rlimit
  try
  {
    auto lim {rlimit{}};

    util::syscall::negative_one_throws_with_errno<void>
      ( ::getrlimit (resource, std::addressof (lim))
      );

    return lim;
  }
  catch (...)
  {
    std::throw_with_nested
      ( mcs::util::syscall::Error
        { fmt::format ("syscall::getrlimit (resource = {})", resource)
        }
      );
  }
}

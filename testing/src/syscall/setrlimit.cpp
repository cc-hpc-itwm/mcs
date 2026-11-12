// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/testing/syscall/setrlimit.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <memory>

namespace mcs::testing::syscall
{
  auto setrlimit (int resource, rlimit limit) -> void
  try
  {
    util::syscall::negative_one_throws_with_errno<void>
      ( ::setrlimit (resource, std::addressof (limit))
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( mcs::util::syscall::Error
        { fmt::format ( "syscall::setrlimit (resource = {}, limit = ({}, {}))"
                      , resource
                      , limit.rlim_cur
                      , limit.rlim_max
                      )
        }
      );
  }
}

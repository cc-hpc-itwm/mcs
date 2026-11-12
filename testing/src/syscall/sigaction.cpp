// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/testing/syscall/sigaction.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/Error.hpp>

namespace mcs::testing::syscall
{
  auto sigaction
    ( int signal
    , struct ::sigaction const* action
    , struct ::sigaction* old_action
    ) -> void
  try
  {
    return util::syscall::negative_one_throws_with_errno<void>
      ( ::sigaction (signal, action, old_action)
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( mcs::util::syscall::Error
        { fmt::format
          ( "syscall::sigaction (signal = {}, action = {}, old_action = {})"
          , signal
          , util::cast<void const*> (action)
          , util::cast<void*> (old_action)
          )
        }
      );
  }
}

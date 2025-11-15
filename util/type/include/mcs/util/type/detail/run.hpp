// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <utility>

namespace mcs::util::type::detail
{
  template<typename T, typename Runner, typename... Args>
    constexpr auto run (Runner&& runner, Args&&... args)
  {
    return std::invoke
      ( &Runner::template operator()<T>
      , std::forward<Runner> (runner)
      , std::forward<Args> (args)...
      );
  }
}

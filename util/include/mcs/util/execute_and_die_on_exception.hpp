// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <functional>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <type_traits>
#include <utility>

namespace mcs::util
{
  template<typename Description, typename Fun, typename... Args>
    requires (std::is_invocable_v<Fun, Args...>)
    auto execute_and_die_on_exception
      ( Description&& description
      , Fun&& fun
      , Args&&... args
      ) noexcept -> std::invoke_result_t<Fun, Args...>
  try
  {
    return std::invoke (std::forward<Fun> (fun), std::forward<Args> (args)...);
  }
  catch (...)
  {
    FMT::print_noexcept
      ( stderr
      , "Abort: {}: {}\n"
      , std::forward<Description> (description)
      , std::current_exception()
      );

    std::abort();
  }
}

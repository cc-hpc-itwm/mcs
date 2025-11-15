// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include "detail/execute_and_die_on_exception_function.hpp"
#include "scope/exit.hpp"
#include "scope/fail.hpp"
#include "scope/success.hpp"
#include <utility>

namespace mcs::nonstd
{
  template<typename Description, typename OnExit>
    [[nodiscard]] auto make_scope_exit_that_dies_on_exception
      ( Description&& description
      , OnExit&& on_exit
      ) noexcept
  {
    return nonstd::make_scope_exit
      ( detail::execute_and_die_on_exception_function
        ( std::forward<Description> (description)
        , std::forward<OnExit> (on_exit)
        )
      );
  }

  template<typename Description, typename OnExit>
    [[nodiscard]] auto make_scope_fail_that_dies_on_exception
      ( Description&& description
      , OnExit&& on_exit
      ) noexcept
  {
    return nonstd::make_scope_fail
      ( detail::execute_and_die_on_exception_function
        ( std::forward<Description> (description)
        , std::forward<OnExit> (on_exit)
        )
      );
  }

  template<typename Description, typename OnExit>
    [[nodiscard]] auto make_scope_success_that_dies_on_exception
      ( Description&& description
      , OnExit&& on_exit
      ) noexcept
  {
    return nonstd::make_scope_success
      ( detail::execute_and_die_on_exception_function
        ( std::forward<Description> (description)
        , std::forward<OnExit> (on_exit)
        )
      );
  }
}

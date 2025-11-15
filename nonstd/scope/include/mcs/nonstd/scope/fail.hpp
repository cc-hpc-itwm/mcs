// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <type_traits>

namespace mcs::nonstd
{
  // Similar to std::experimental::scope_fail, except
  // - is neither move constructible nor move assignable
  // - requires the exit function to always be nothrow_invocable
  // - requires the exit function to always be nothrow_move_constructible
  //
  template<typename OnFail>
    requires (  std::is_nothrow_invocable_v<OnFail>
             && std::is_nothrow_move_constructible_v<OnFail>
             )
    struct scope_fail
    {
      [[nodiscard]] scope_fail (OnFail) noexcept;
      scope_fail (scope_fail const&) = delete;
      scope_fail (scope_fail&&) = delete;
      auto operator= (scope_fail const&) -> scope_fail& = delete;
      auto operator= (scope_fail&&) -> scope_fail& = delete;

      ~scope_fail() noexcept;
      constexpr auto release() noexcept;

    private:
      bool _released {false};
      int _uncaught_exceptions {std::uncaught_exceptions()};
      OnFail _on_fail;
    };
}

namespace mcs::nonstd
{
  template<typename OnFail>
    requires (  std::is_nothrow_invocable_v<OnFail>
             && std::is_nothrow_move_constructible_v<OnFail>
             )
    [[nodiscard]] auto make_scope_fail (OnFail);
}

#include "detail/fail.ipp"

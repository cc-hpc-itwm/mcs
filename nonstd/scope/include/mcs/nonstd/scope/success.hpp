// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <type_traits>

namespace mcs::nonstd
{
  // Similar to std::experimental::scope_success, except
  // - is neither move constructible nor move assignable
  // - requires the exit function to always be nothrow_invocable
  // - requires the exit function to always be nothrow_move_constructible
  //
  template<typename OnSuccess>
    requires (  std::is_nothrow_invocable_v<OnSuccess>
             && std::is_nothrow_move_constructible_v<OnSuccess>
             )
    struct scope_success
    {
      [[nodiscard]] scope_success (OnSuccess) noexcept;
      scope_success (scope_success const&) = delete;
      scope_success (scope_success&&) = delete;
      auto operator= (scope_success const&) -> scope_success& = delete;
      auto operator= (scope_success&&) -> scope_success& = delete;

      ~scope_success() noexcept;
      constexpr auto release() noexcept;

    private:
      bool _released {false};
      int _uncaught_exceptions {std::uncaught_exceptions()};
      OnSuccess _on_success;
    };
}

namespace mcs::nonstd
{
  template<typename OnSuccess>
    requires (  std::is_nothrow_invocable_v<OnSuccess>
             && std::is_nothrow_move_constructible_v<OnSuccess>
             )
    [[nodiscard]] auto make_scope_success (OnSuccess);
}

#include "detail/success.ipp"

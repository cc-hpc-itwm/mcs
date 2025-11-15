// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <type_traits>

namespace mcs::nonstd
{
  // Similar to std::experimental::scope_exit, except
  // - is neither move constructible nor move assignable
  // - requires the exit function to always be nothrow_invocable
  // - requires the exit function to always be nothrow_move_constructible
  //
  template<typename OnExit>
    requires (  std::is_nothrow_invocable_v<OnExit>
             && std::is_nothrow_move_constructible_v<OnExit>
             )
    struct scope_exit
    {
      [[nodiscard]] scope_exit (OnExit) noexcept;
      scope_exit (scope_exit const&) = delete;
      scope_exit (scope_exit&&) = delete;
      auto operator= (scope_exit const&) -> scope_exit& = delete;
      auto operator= (scope_exit&&) -> scope_exit& = delete;

      ~scope_exit() noexcept;
      constexpr auto release() noexcept -> void;

    private:
      bool _released {false};
      OnExit _on_exit;
    };
}

namespace mcs::nonstd
{
  template<typename OnExit>
    requires (  std::is_nothrow_invocable_v<OnExit>
             && std::is_nothrow_move_constructible_v<OnExit>
             )
    [[nodiscard]] auto make_scope_exit (OnExit);
}

#include "detail/exit.ipp"

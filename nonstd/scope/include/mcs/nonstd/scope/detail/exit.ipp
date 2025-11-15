// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <utility>

namespace mcs::nonstd
{
  template<typename OnExit>
    requires (  std::is_nothrow_invocable_v<OnExit>
             && std::is_nothrow_move_constructible_v<OnExit>
             )
    scope_exit<OnExit>::scope_exit (OnExit on_exit) noexcept
      : _on_exit {std::move (on_exit)}
  {}

  template<typename OnExit>
    requires (  std::is_nothrow_invocable_v<OnExit>
             && std::is_nothrow_move_constructible_v<OnExit>
             )
    scope_exit<OnExit>::~scope_exit() noexcept
  {
    if (!_released)
    {
      std::invoke (_on_exit);
    }
  }

  template<typename OnExit>
    requires (  std::is_nothrow_invocable_v<OnExit>
             && std::is_nothrow_move_constructible_v<OnExit>
             )
    constexpr auto scope_exit<OnExit>::release() noexcept -> void
  {
    _released = true;
  }
}

namespace mcs::nonstd
{
  template<typename OnExit>
    requires (  std::is_nothrow_invocable_v<OnExit>
             && std::is_nothrow_move_constructible_v<OnExit>
             )
    auto make_scope_exit (OnExit on_exit)
  {
    return scope_exit<OnExit> {std::move (on_exit)};
  }
}

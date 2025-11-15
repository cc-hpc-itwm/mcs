// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <utility>

namespace mcs::nonstd
{
  template<typename OnSuccess>
    requires (  std::is_nothrow_invocable_v<OnSuccess>
             && std::is_nothrow_move_constructible_v<OnSuccess>
             )
    scope_success<OnSuccess>::scope_success (OnSuccess on_success) noexcept
      : _on_success {std::move (on_success)}
  {}

  template<typename OnSuccess>
    requires (  std::is_nothrow_invocable_v<OnSuccess>
             && std::is_nothrow_move_constructible_v<OnSuccess>
             )
    scope_success<OnSuccess>::~scope_success() noexcept
  {
    if (!_released)
    {
      if (_uncaught_exceptions == std::uncaught_exceptions())
      {
        std::invoke (_on_success);
      }
    }
  }

  template<typename OnSuccess>
    requires (  std::is_nothrow_invocable_v<OnSuccess>
             && std::is_nothrow_move_constructible_v<OnSuccess>
             )
    constexpr auto scope_success<OnSuccess>::release() noexcept
  {
    _released = true;
  }
}

namespace mcs::nonstd
{
  template<typename OnSuccess>
    requires (  std::is_nothrow_invocable_v<OnSuccess>
             && std::is_nothrow_move_constructible_v<OnSuccess>
             )
    auto make_scope_success (OnSuccess on_success)
  {
    return scope_success<OnSuccess> {std::move (on_success)};
  }
}

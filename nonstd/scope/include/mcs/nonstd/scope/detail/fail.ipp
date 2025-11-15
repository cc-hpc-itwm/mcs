// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <utility>

namespace mcs::nonstd
{
  template<typename OnFail>
    requires (  std::is_nothrow_invocable_v<OnFail>
             && std::is_nothrow_move_constructible_v<OnFail>
             )
    scope_fail<OnFail>::scope_fail (OnFail on_fail) noexcept
      : _on_fail {std::move (on_fail)}
  {}

  template<typename OnFail>
    requires (  std::is_nothrow_invocable_v<OnFail>
             && std::is_nothrow_move_constructible_v<OnFail>
             )
    scope_fail<OnFail>::~scope_fail() noexcept
  {
    if (!_released)
    {
      if (_uncaught_exceptions != std::uncaught_exceptions())
      {
        std::invoke (_on_fail);
      }
    }
  }

  template<typename OnFail>
    requires (  std::is_nothrow_invocable_v<OnFail>
             && std::is_nothrow_move_constructible_v<OnFail>
             )
    constexpr auto scope_fail<OnFail>::release() noexcept
  {
    _released = true;
  }
}
namespace mcs::nonstd
{
  template<typename OnFail>
    requires (  std::is_nothrow_invocable_v<OnFail>
             && std::is_nothrow_move_constructible_v<OnFail>
             )
    auto make_scope_fail (OnFail on_fail)
  {
    return scope_fail<OnFail> {std::move (on_fail)};
  }
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <type_traits>

namespace mcs::util
{
  template<long, typename> struct Timer;
}

namespace mcs::util::timer
{
  template<long ClockScale>
    struct Ticks
  {
    [[nodiscard]] constexpr auto clock_scale() const noexcept -> long;
    [[nodiscard]] operator long() const noexcept;
    [[nodiscard]] auto value() const noexcept;

    template<std::integral I>
      [[nodiscard]] constexpr auto per_second (I) const -> long;

  private:
    template<long, typename> friend struct mcs::util::Timer;

    [[nodiscard]] constexpr Ticks (long) noexcept;

    long _value;
  };
}

namespace fmt
{
  template<long ClockScale>
    struct formatter<mcs::util::timer::Ticks<ClockScale>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::util::timer::Ticks<ClockScale> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/Ticks.ipp"

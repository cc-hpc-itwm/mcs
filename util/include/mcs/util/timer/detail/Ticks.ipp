// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <limits>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/cast.hpp>
#include <utility>

namespace mcs::util::timer
{
  template<long ClockScale>
    constexpr Ticks<ClockScale>::Ticks (long value) noexcept
      : _value {value}
  {}

  template<long ClockScale>
    constexpr auto Ticks<ClockScale>::clock_scale() const noexcept -> long
  {
    return ClockScale;
  }

  template<long ClockScale>
    Ticks<ClockScale>::operator long() const noexcept
  {
    return value();
  }

  template<long ClockScale>
    auto Ticks<ClockScale>::value() const noexcept
  {
    return _value;
  }

  template<long ClockScale>
    template<std::integral I>
      constexpr auto Ticks<ClockScale>::per_second (I i) const -> long
  {
    return std::cmp_equal (_value, 0)
      ? std::numeric_limits<long>::infinity()
      : mcs::util::cast<long> (i) * ClockScale / _value
      ;
  }
}

namespace fmt
{
  template<long ClockScale>
    MCS_UTIL_FMT_DEFINE_PARSE (context, mcs::util::timer::Ticks<ClockScale>)
  {
    return context.begin();
  }
  template<long ClockScale>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( ticks
      , context
      , mcs::util::timer::Ticks<ClockScale>
      )
  {
    return fmt::format_to
      ( context.out()
      , "{} sec/{}"
      , ticks.value()
      , ticks.clock_scale()
      );
  }
}

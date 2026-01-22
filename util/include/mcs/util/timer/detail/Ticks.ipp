// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <limits>
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
    template<typename ParseContext>
      constexpr auto formatter<mcs::util::timer::Ticks<ClockScale>>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<long ClockScale>
    template<typename FormatContext>
      constexpr auto formatter<mcs::util::timer::Ticks<ClockScale>>::format
        ( mcs::util::timer::Ticks<ClockScale> const& ticks
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "{} sec/{}"
      , ticks.value()
      , ticks.clock_scale()
      );
  }
}

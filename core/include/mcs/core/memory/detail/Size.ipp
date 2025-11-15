// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/divru.hpp>
#include <mcs/util/read/STD/tuple.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>
#include <type_traits>

namespace mcs::core::memory
{
  template<std::unsigned_integral I>
    constexpr Size::Size (I value) noexcept
      : _value {value}
  {}

  template<std::signed_integral I>
    constexpr Size::Size (I value)
      : Size {util::cast<underlying_type> (value)}
  {}

  template<std::integral I>
    constexpr auto make_size
      ( I i
      ) noexcept (std::unsigned_integral<I>) -> Size
  {
    return Size {i};
  }

  constexpr auto Size::operator+= (Size const& other) noexcept -> Size&
  {
    _value += other._value;

    return *this;
  }

  constexpr auto Size::operator-= (Size const& other) noexcept -> Size&
  {
    _value -= other._value;

    return *this;
  }
}

namespace mcs::core::memory
{
  [[nodiscard]] constexpr auto operator+
    ( Size const& lhs
    , Size const& rhs
    ) noexcept -> Size
  {
    return make_size (lhs._value + rhs._value);
  }

  constexpr auto operator/
    ( Size const& lhs
    , Size const& rhs
    ) -> Size::underlying_type
  {
    return lhs._value / rhs._value;
  }

  constexpr auto operator%
    ( Size const& lhs
    , Size const& rhs
    ) -> Size::underlying_type
  {
    return lhs._value % rhs._value;
  }

  constexpr auto divru
    ( Size const& lhs
    , Size const& rhs
    ) -> Size::underlying_type
  {
    return util::divru (lhs._value, rhs._value);
  }

  constexpr auto operator- (Size lhs, Size rhs) -> Size
  {
    return make_size (lhs._value - rhs._value);
  }

  template<std::unsigned_integral T>
    constexpr auto operator*
      ( T const& x
      , Size const& size
      ) noexcept -> Size
  {
    return make_size (x * size._value);
  }

  template<std::unsigned_integral T>
    constexpr auto operator*
      ( Size const& size
      , T const& x
      ) noexcept -> Size
  {
    return make_size (size._value * x);
  }

  template<std::unsigned_integral I>
    requires (sizeof (I) >= sizeof (Size::underlying_type))
    auto size_cast (Size const& size) -> I
  {
    if constexpr (std::is_same_v<I, Size::underlying_type>)
    {
      return size._value;
    }
    else
    {
      return util::cast<I> (size._value);
    }
  }
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, mcs::core::memory::Size)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (size, ctx, mcs::core::memory::Size)
  {
    return fmt::format_to (ctx.out(), "sz_{}", size._value);
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, core::memory::Size)
  {
    maybe_prefix (state, "sz_");

    return core::memory::make_size
      (parse<core::memory::Size::underlying_type> (state));
  }
}

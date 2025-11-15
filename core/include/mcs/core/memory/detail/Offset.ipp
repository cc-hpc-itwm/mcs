// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/read/STD/tuple.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>

namespace mcs::core::memory
{
  template<std::unsigned_integral I>
    constexpr Offset::Offset (I value) noexcept
      : _value {value}
  {}

  template<std::signed_integral I>
    constexpr Offset::Offset (I value)
      : Offset {util::cast<underlying_type> (value)}
  {}

  template<std::integral I>
    constexpr auto make_offset
      ( I i
      ) noexcept (std::unsigned_integral<I>) -> Offset
  {
    return Offset {i};
  }
}

namespace mcs::core::memory
{
  constexpr auto Offset::operator+= (Size const& size) -> Offset&
  {
    return *this = *this + size;
  }

  constexpr auto operator-
    ( Offset const& lhs
    , Offset const& rhs
    ) -> Size
  {
    return make_size (lhs._value - rhs._value);
  }

  constexpr auto operator+
    ( Offset const& lhs
    , Size const& rhs
    ) -> Offset
  {
    return make_offset (lhs._value + rhs._value);
  }
  constexpr auto operator+ (Offset lhs, Offset rhs) -> Offset
  {
    return make_offset (lhs._value + rhs._value);
  }
  constexpr auto operator-
    ( Offset const& lhs
    , Size const& rhs
    ) -> Offset
  {
    return make_offset (lhs._value - rhs._value);
  }

  template<typename T>
    constexpr auto operator+
      ( T* p
      , Offset o
      ) -> T*
  {
    return p + o._value;
  }

  constexpr auto make_off_t (Offset o) -> off_t
  {
    return util::cast<off_t> (o._value);
  }
}

namespace mcs::core::memory
{
  template<std::unsigned_integral I>
    requires (sizeof (I) >= sizeof (Offset::underlying_type))
    auto offset_cast (Offset const& offset) -> I
  {
    if constexpr (std::is_same_v<I, Offset::underlying_type>)
    {
      return offset._value;
    }
    else
    {
      return util::cast<I> (offset._value);
    }
  }
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, mcs::core::memory::Offset)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (size, ctx, mcs::core::memory::Offset)
  {
    return fmt::format_to (ctx.out(), "of_{}", size._value);
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, core::memory::Offset)
  {
    maybe_prefix (state, "of_");

    return core::memory::make_offset
      (parse<core::memory::Offset::underlying_type> (state));
  }
}

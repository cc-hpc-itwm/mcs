// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/cast.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/STD/tuple.hpp>
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

  constexpr auto operator- (Offset offset) -> Offset
  {
    return make_offset (-offset._value);
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
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::memory::Offset>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::memory::Offset>::format
      ( mcs::core::memory::Offset const& size
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "of_{}", size._value);
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<core::memory::Offset>::read
      ( State<Char>& state
      ) -> core::memory::Offset
  {
    maybe_prefix (state, "of_");

    return core::memory::make_offset
      (parse<core::memory::Offset::underlying_type> (state));
  }
}

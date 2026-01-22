// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/cast.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace mcs::block_device::block
{
  template<std::unsigned_integral I>
    constexpr Count::Count (I value) noexcept
      : _value {value}
  {}

  template<std::signed_integral I>
    constexpr Count::Count (I value)
      : Count {util::cast<underlying_type> (value)}
  {}

  template<std::integral I>
    constexpr auto make_count
      ( I i
      ) noexcept (std::unsigned_integral<I>) -> Count
  {
    return Count {i};
  }

  constexpr auto Count::operator+= (Count const& other) noexcept -> Count&
  {
    _value += other._value;

    return *this;
  }

  constexpr auto Count::operator-= (Count const& other) noexcept -> Count&
  {
    _value -= other._value;

    return *this;
  }
}

namespace mcs::block_device::block
{
  constexpr auto operator+
    ( Count const& lhs
    , Count const& rhs
    ) noexcept -> Count
  {
    return make_count (lhs._value + rhs._value);
  }

  constexpr auto operator-
    ( Count const& lhs
    , Count const& rhs
    ) noexcept -> Count
  {
    return make_count (lhs._value - rhs._value);
  }

  constexpr auto operator*
    ( Count const& count
    , Size const& size
    ) noexcept -> core::memory::Size
  {
    return core::memory::make_size (count._value * size._value);
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::block_device::block::Count>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::block_device::block::Count>::format
      ( mcs::block_device::block::Count const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::block_device::block::Count "
      , std::make_tuple (value._value)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::block_device::block::Count>::read
      ( State<Char>& state
      ) -> mcs::block_device::block::Count
  {
    prefix (state, "mcs::block_device::block::Count ");

    return std::make_from_tuple<mcs::block_device::block::Count>
      (parse<std::tuple<decltype (mcs::block_device::block::Count::_value)>> (state));
  }
}

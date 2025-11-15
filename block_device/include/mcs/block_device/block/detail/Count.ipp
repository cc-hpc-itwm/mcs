// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/cast.hpp>
#include <mcs/util/tuplish/define.hpp>

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

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "mcs::block_device::block::Count "
  , mcs::block_device::block::Count
  , _value
  );

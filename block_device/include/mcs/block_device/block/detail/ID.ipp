// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/cast.hpp>
#include <mcs/util/tuplish/define.hpp>

namespace mcs::block_device::block
{
  template<std::unsigned_integral I>
    constexpr ID::ID (I value) noexcept
      : _value {value}
  {}

  template<std::signed_integral I>
    constexpr ID::ID (I value)
      : ID {util::cast<underlying_type> (value)}
  {}

  template<std::integral I>
    [[nodiscard]] constexpr auto make_id
      ( I i
      ) noexcept (std::unsigned_integral<I>) -> ID
  {
    return ID {i};
  }

  constexpr auto ID::operator+= (Count const& x) noexcept -> ID&
  {
    _value += x._value;

    return *this;
  }

  constexpr auto ID::operator++() noexcept -> ID&
  {
    ++_value;
    return *this;
  }

  constexpr auto operator+
    ( ID const& id
    , Count const& x
    ) noexcept -> ID
  {
    return ID {id._value + x._value};
  }

  constexpr auto operator-
    ( ID const& lhs
    , ID const& rhs
    ) noexcept -> Count
  {
    return make_count (lhs._value - rhs._value);
  }
}

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "mcs::block_device::block::ID "
  , mcs::block_device::block::ID
  , _value
  );

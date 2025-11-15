// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/cast.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <utility>

namespace mcs::block_device::block
{
  template<std::unsigned_integral I>
    constexpr Size::Size (I value)
      : _value {value}
  {
    if (std::cmp_equal (value, 0))
    {
      throw Error::MustNotBeZero{};
    }
  }

  template<std::signed_integral I>
    constexpr Size::Size (I value)
      : Size {util::cast<underlying_type> (value)}
  {}

  template<std::integral I>
    constexpr auto make_size (I i) -> Size
  {
    return Size {i};
  }

  constexpr auto operator/
    ( core::memory::Size size
    , Size block_size
    ) noexcept -> core::memory::Size::underlying_type
  {
    return size / core::memory::make_size (block_size._value);
  }
}

namespace mcs::core::memory
{
  constexpr auto make_size
    ( block_device::block::Size const& block_size
    ) noexcept -> Size
  {
    return make_size (block_size._value);
  }
}

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "mcs::block_device::block::Size "
  , mcs::block_device::block::Size
  , _value
  );

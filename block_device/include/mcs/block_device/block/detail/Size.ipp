// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/cast.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>
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

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::block_device::block::Size>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::block_device::block::Size>::format
      ( mcs::block_device::block::Size const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::block_device::block::Size "
      , std::make_tuple (value._value)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::block_device::block::Size>::read
      ( State<Char>& state
      ) -> mcs::block_device::block::Size
  {
    prefix (state, "mcs::block_device::block::Size ");

    return std::make_from_tuple<mcs::block_device::block::Size>
      (parse<std::tuple<decltype (mcs::block_device::block::Size::_value)>> (state));
  }
}

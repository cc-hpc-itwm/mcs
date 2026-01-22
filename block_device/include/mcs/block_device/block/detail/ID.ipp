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

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::block_device::block::ID>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::block_device::block::ID>::format
      ( mcs::block_device::block::ID const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::block_device::block::ID "
      , std::make_tuple (value._value)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::block_device::block::ID>::read
      ( State<Char>& state
      ) -> mcs::block_device::block::ID
  {
    prefix (state, "mcs::block_device::block::ID ");

    return std::make_from_tuple<mcs::block_device::block::ID>
      (parse<std::tuple<decltype (mcs::block_device::block::ID::_value)>> (state));
  }
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>

namespace mcs::core::transport::client
{
  constexpr ID::ID (underlying_type value) noexcept
    : _value {value}
  {}

  constexpr auto ID::operator++() noexcept -> ID&
  {
    ++_value;

    return *this;
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::transport::client::ID>::parse
      (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::transport::client::ID>::format
      ( mcs::core::transport::client::ID const& id
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "pi_{}", id._value);
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<core::transport::client::ID>::read
      ( State<Char>& state
      ) -> core::transport::client::ID
  {
    maybe_prefix (state, "pi_");

    return core::transport::client::ID
      {parse<core::transport::client::ID::underlying_type> (state)};
  }
}

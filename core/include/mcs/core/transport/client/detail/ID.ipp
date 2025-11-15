// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/define.hpp>
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
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, mcs::core::transport::client::ID)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (id, ctx, mcs::core::transport::client::ID)
  {
    return fmt::format_to (ctx.out(), "pi_{}", id._value);
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    ( state, core::transport::client::ID
    )
  {
    maybe_prefix (state, "pi_");

    return core::transport::client::ID
      {parse<core::transport::client::ID::underlying_type> (state)};
  }
}

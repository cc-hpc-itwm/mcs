// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>

namespace mcs::core::storage
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
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, mcs::core::storage::ID)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (id, ctx, mcs::core::storage::ID)
  {
    return fmt::format_to (ctx.out(), "bi_{}", id._value);
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, core::storage::ID)
  {
    maybe_prefix (state, "bi_");

    return core::storage::ID
      {parse<core::storage::ID::underlying_type> (state)};
  }
}

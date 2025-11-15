// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/prefix.hpp>

namespace mcs::iov_backend::storage
{
  constexpr ID::ID (underlying_type value) noexcept
    : _value {value}
  {}
}

namespace mcs::iov_backend::storage
{
  constexpr auto ID::operator++() noexcept -> ID&
  {
    ++_value;

    return *this;
  }
}
namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, mcs::iov_backend::storage::ID)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (id, ctx, mcs::iov_backend::storage::ID)
  {
    return fmt::format_to (ctx.out(), "si_{}", id._value);
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    ( state
    , iov_backend::storage::ID
    )
  {
    // \todo tuplish with optional prefix
    maybe_prefix (state, "si_");

    return iov_backend::storage::ID
      {parse<iov_backend::storage::ID::underlying_type> (state)};
  }
}

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace mcs::rpc::multi_client::detail
{
  constexpr auto CallID::operator++() noexcept -> CallID&
  {
    ++_value;

    return *this;
  }
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, mcs::rpc::multi_client::detail::CallID)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (id, ctx, mcs::rpc::multi_client::detail::CallID)
  {
    return fmt::format_to (ctx.out(), "ci_{}", id._value);
  }
}

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstdint>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/hash/access.hpp>
#include <mcs/util/hash/declare.hpp>

namespace mcs::rpc::multi_client::detail
{
  struct CallID
  {
    constexpr auto operator<=> (CallID const&) const noexcept = default;

    [[nodiscard]] constexpr CallID() noexcept = default;

    constexpr auto operator++() noexcept -> CallID&;

  private:
    using underlying_type = std::uintmax_t;

    underlying_type _value {0u};

    MCS_UTIL_FMT_ACCESS();
    MCS_UTIL_HASH_ACCESS();
  };
}

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (mcs::rpc::multi_client::detail::CallID);
}

namespace std
{
  template<> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE
    (mcs::rpc::multi_client::detail::CallID);
}

#include "detail/CallID.ipp"

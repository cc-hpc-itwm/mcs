// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstdint>
#include <fmt/base.h>
#include <functional>

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

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct std::hash;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::rpc::multi_client::detail::CallID>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::rpc::multi_client::detail::CallID const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace std
{
  template<>
    struct hash<mcs::rpc::multi_client::detail::CallID>
  {
    auto operator()
      ( mcs::rpc::multi_client::detail::CallID
      ) const noexcept -> size_t
      ;

  private:
    hash<mcs::rpc::multi_client::detail::CallID::underlying_type> _hash;
  };
}

#include "detail/CallID.ipp"

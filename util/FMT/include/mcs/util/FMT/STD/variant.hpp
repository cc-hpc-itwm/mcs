// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <variant>

namespace fmt
{
  template<formattable... Ts>
    struct formatter<std::variant<Ts...>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( std::variant<Ts...> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/variant.ipp"

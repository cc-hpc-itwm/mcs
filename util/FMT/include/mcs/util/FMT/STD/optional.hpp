// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <optional>

namespace fmt
{
  template<formattable T>
    struct formatter<std::optional<T>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( std::optional<T> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/optional.ipp"

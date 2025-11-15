// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  template<formattable... Ts>
    MCS_UTIL_FMT_DEFINE_PARSE (ctx, std::variant<Ts...>)
  {
    return ctx.begin();
  }
  template<formattable... Ts>
    MCS_UTIL_FMT_DEFINE_FORMAT (variant, ctx, std::variant<Ts...>)
  {
    return std::visit
      ( [&] (auto const& value)
        {
          return fmt::format_to (ctx.out(), "{}", value);
        }
     , variant
     );
  }
}

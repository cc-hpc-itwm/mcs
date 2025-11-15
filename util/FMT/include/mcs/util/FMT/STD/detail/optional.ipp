// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/FMT/define.hpp>
#include <tuple>

namespace fmt
{
  template<formattable T>
    MCS_UTIL_FMT_DEFINE_PARSE (ctx, std::optional<T>)
  {
    return ctx.begin();
  }
  template<formattable T>
    MCS_UTIL_FMT_DEFINE_FORMAT (opt, ctx, std::optional<T>)
  {
    if (opt.has_value())
    {
      return fmt::format_to
        ( ctx.out()
        , "Just {}"
        , std::make_tuple (opt.value())
        );
    }

    return fmt::format_to (ctx.out(), "Nothing");
  }
}

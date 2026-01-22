// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <tuple>

namespace fmt
{
  template<formattable T>
    template<typename ParseContext>
      constexpr auto formatter<std::optional<T>>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<formattable T>
    template<typename FormatContext>
      constexpr auto formatter<std::optional<T>>::format
        ( std::optional<T> const& opt
        , FormatContext& ctx
        ) const -> decltype (ctx.out())
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

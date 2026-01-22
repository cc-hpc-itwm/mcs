// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace fmt
{
  template<formattable... Ts>
    template<typename ParseContext>
      constexpr auto formatter<std::variant<Ts...>>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<formattable... Ts>
    template<typename FormatContext>
      constexpr auto formatter<std::variant<Ts...>>::format
        ( std::variant<Ts...> const& variant
        , FormatContext& ctx
        ) const -> decltype (ctx.out())
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

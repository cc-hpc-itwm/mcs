// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <type_traits>

namespace fmt
{
  template<formattable... Ts>
    template<typename ParseContext>
      constexpr auto formatter<mcs::util::FMT::STD::Variant<Ts...>>::parse
        ( ParseContext& context
        )
  {
    return context.begin();
  }
  template<formattable... Ts>
    template<typename FormatContext>
      constexpr auto formatter<mcs::util::FMT::STD::Variant<Ts...>>::format
        ( mcs::util::FMT::STD::Variant<Ts...> const& variant
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return std::visit
      ( [&] (auto const& value)
        {
          return fmt::format_to (context.out(), "{}", value);
        }
      , variant._value
      );
  }
}

namespace mcs::util::FMT::STD
{
  template<typename... Ts>
    constexpr Variant<Ts...>::Variant (std::variant<Ts...> variant) noexcept
      : _value {std::move (variant)}
  {
    static_assert
      ( std::is_nothrow_move_constructible_v<std::variant<Ts...>>
      );
  }
}

namespace mcs::util::FMT::STD::VARIANT
{
  template<typename... Ts>
    auto format::operator()
      ( std::variant<Ts...> const& variant
      ) const -> std::string
  {
    return fmt::format ("{}", Variant {variant});
  }
}

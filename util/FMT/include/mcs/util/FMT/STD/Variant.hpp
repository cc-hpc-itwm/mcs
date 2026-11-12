// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <string>
#include <variant>

namespace mcs::util::FMT::STD
{
  template<typename... Ts>
    struct Variant
  {
    [[nodiscard]] constexpr Variant (std::variant<Ts...>) noexcept;

  private:
    friend struct fmt::formatter<Variant<Ts...>>;

    std::variant<Ts...> _value;
  };

  namespace VARIANT
  {
    struct format
    {
      template<typename... Ts>
        auto operator()
          ( std::variant<Ts...> const&
          ) const -> std::string
        ;
    };
  }
}

namespace fmt
{
  template<formattable... Ts>
    struct formatter<mcs::util::FMT::STD::Variant<Ts...>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::util::FMT::STD::Variant<Ts...> const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

#include "detail/Variant.ipp"

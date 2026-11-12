// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <optional>
#include <string>

namespace mcs::util::FMT::STD
{
  template<typename T>
    struct Optional
  {
    [[nodiscard]] constexpr Optional (std::optional<T>) noexcept;

  private:
    friend struct fmt::formatter<Optional<T>>;

    std::optional<T> _value;
  };

  namespace OPTIONAL
  {
    struct format
    {
      template<typename T>
        auto operator()
          ( std::optional<T> const&
          ) const -> std::string
        ;
    };
  }
}

namespace fmt
{
  template<formattable T>
    struct formatter<mcs::util::FMT::STD::Optional<T>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::util::FMT::STD::Optional<T> const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

#include "detail/Optional.ipp"

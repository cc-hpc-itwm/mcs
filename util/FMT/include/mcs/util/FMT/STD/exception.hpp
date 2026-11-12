// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <fmt/base.h>

namespace fmt
{
  template<>
    struct formatter<std::exception_ptr>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( std::exception_ptr const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };

  template<>
    struct formatter<std::exception>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( std::exception const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

#include "detail/exception.ipp"

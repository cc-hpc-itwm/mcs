// Copyright (C) 2023-2025 Fraunhofer ITWM
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
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };

  template<>
    struct formatter<std::exception>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( std::exception const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/exception.ipp"

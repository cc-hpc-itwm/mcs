// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <fmt/base.h>

namespace fmt
{
  template<>
    struct formatter<std::filesystem::path>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( std::filesystem::path const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/path.ipp"

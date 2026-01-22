// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>

namespace mcs::core::storage::trace::event::size::max
{
  struct Result
  {
    core::storage::MaxSize _max_size;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::trace::event::size::max::Result>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::trace::event::size::max::Result const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/Result.ipp"

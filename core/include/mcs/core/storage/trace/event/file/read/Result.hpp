// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/memory/Size.hpp>

namespace mcs::core::storage::trace::event::file::read
{
  struct Result
  {
    core::memory::Size _size;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::trace::event::file::read::Result>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::trace::event::file::read::Result const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/Result.ipp"

// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/storage/segment/ID.hpp>

namespace mcs::core::storage::trace::event::segment::create
{
  struct Result
  {
    core::storage::segment::ID segment_id;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::trace::event::segment::create::Result>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::trace::event::segment::create::Result const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

#include "detail/Result.ipp"

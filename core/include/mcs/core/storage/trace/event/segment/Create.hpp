// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>

namespace mcs::core::storage::trace::event::segment
{
  template<core::storage::is_implementation Storage>
    struct Create
  {
    typename Storage::Parameter::Segment::Create parameter_segment_create;
    core::memory::Size size;
  };
}

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    struct formatter<mcs::core::storage::trace::event::segment::Create<Storage>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::trace::event::segment::Create<Storage> const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

#include "detail/Create.ipp"

// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/Concepts.hpp>

namespace mcs::core::storage::implementation::trace
{
  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    struct Tag{};
}

namespace fmt
{
  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    struct formatter<mcs::core::storage::implementation::trace::Tag<Tracer, Storage>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::trace::Tag<Tracer, Storage> const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

#include "detail/Tag.ipp"

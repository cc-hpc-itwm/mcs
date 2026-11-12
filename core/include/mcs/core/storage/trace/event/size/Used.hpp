// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/storage/Concepts.hpp>

namespace mcs::core::storage::trace::event::size
{
  template<core::storage::is_implementation Storage>
    struct Used
  {
    typename Storage::Parameter::Size::Used parameter_size_used;
  };
}

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    struct formatter<mcs::core::storage::trace::event::size::Used<Storage>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::trace::event::size::Used<Storage> const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

#include "detail/Used.ipp"

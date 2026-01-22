// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/storage/Concepts.hpp>

namespace mcs::core::storage::trace::event::size
{
  template<core::storage::is_implementation Storage>
    struct Max
  {
    typename Storage::Parameter::Size::Max _parameter_size_max;
  };
}

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    struct formatter<mcs::core::storage::trace::event::size::Max<Storage>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::trace::event::size::Max<Storage> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/Max.ipp"

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::core::storage::tracer::record
{
  struct Tag{};
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::tracer::record::Tag>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::tracer::record::Tag const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

static_assert
  ( mcs::serialization::is_serializable
      < mcs::core::storage::tracer::record::Tag
      >
  );

#include "detail/Tag.ipp"

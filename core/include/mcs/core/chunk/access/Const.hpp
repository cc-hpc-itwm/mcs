// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <span>

namespace mcs::core::chunk::access
{
  struct Const
  {
    template<typename T>
      using ValueType = T const;

    template<typename T>
      using Pointer = ValueType<T>*;

    template<typename T>
      using Span = std::span<ValueType<T>>;

    template<typename T>
      struct Value
    {
      constexpr explicit Value (T);

      T _value;
    };
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::chunk::access::Const>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::chunk::access::Const const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/Const.ipp"

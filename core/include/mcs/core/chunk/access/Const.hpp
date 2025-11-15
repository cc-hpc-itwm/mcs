// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/FMT/declare.hpp>
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
  template<> MCS_UTIL_FMT_DECLARE (mcs::core::chunk::access::Const);
}

#include "detail/Const.ipp"

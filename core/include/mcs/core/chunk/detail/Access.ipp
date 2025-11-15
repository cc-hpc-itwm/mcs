// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::core::chunk
{
  template<is_access A, typename T>
    constexpr auto select ( typename access::Const::Value<T> c
                          , typename access::Mutable::Value<T> m
                          ) -> T
  {
    if constexpr (std::is_same_v<A, access::Const>)
    {
      return c._value;
    }
    else
    {
      return m._value;
    }
  }

  template<is_access A, typename T>
    constexpr auto make_value (T value) -> typename A::template Value<T>
  {
    return typename A::template Value<T> {std::move (value)};
  }
}

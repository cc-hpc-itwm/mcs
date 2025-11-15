// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstddef>
#include <type_traits>

namespace mcs::util::type
{
  template<typename T, typename... Ts>
    struct Index<T, T, Ts...>
      : std::integral_constant<std::size_t, 0>
  {};

  template<typename T, typename U, typename... Ts>
    struct Index<T, U, Ts...>
      : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value>
  {};
}

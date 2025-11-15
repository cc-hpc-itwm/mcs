// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstddef>
#include <type_traits>

namespace mcs::util::type
{
  template<>
    struct Size<>
      : std::integral_constant<std::size_t, 0>
  {};

  template<typename Head, typename... Tail>
    struct Size<Head, Tail...>
      : std::integral_constant<std::size_t, 1 + Size<Tail...>::value>
  {};
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util::type
{
  // The (minimal) index of a type in a list of types.
  //
  // EXAMPLE:
  //   static_assert (Index<int, int, char, int>::value == 0);
  //   static_assert (Index<char, int, char, int>::value == 1);
  //
  // Fails to compile if T is not included in the Ts...
  //
  template<typename T, typename... Ts> struct Index;
}

#include "detail/Index.ipp"

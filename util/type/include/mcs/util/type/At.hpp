// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>

namespace mcs::util::type
{
  // The n-th type of a list of types.
  //
  // EXAMPLE:
  //   static_assert (std::is_same_v<At<0, int, char>::type, int>)
  //   static_assert (std::is_same_v<At<1, int, char>::type, char>)
  //
  // Fails to compile if the index is ouf of range.
  //
  template<std::size_t, typename...> struct At;
}

#include "detail/At.ipp"

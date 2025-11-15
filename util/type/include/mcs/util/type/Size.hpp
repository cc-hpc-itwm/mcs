// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>

namespace mcs::util::type
{
  // The size of a list of types.
  //
  // EXAMPLE:
  //   static_assert (Size<int, char>::value, 2>)
  //   static_assert (Size<>::value, 0>)
  //
  template<typename...> struct Size;
}

#include "detail/Size.ipp"

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>

namespace mcs::serialization::detail::tag::STD
{
  struct Variant
  {
    std::size_t index;
  };
}

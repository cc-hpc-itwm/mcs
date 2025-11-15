// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>

namespace mcs::serialization::detail::tag::STD
{
  struct UnorderedMap
  {
    std::size_t size;
    std::size_t bucket_count;
  };
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>

namespace mcs::serialization::detail::tag::STD
{
  struct UnorderedSet
  {
    std::size_t size;
    std::size_t bucket_count;
  };
}

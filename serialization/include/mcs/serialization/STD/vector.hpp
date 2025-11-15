// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/declare.hpp>
#include <vector>

namespace mcs::serialization
{
  template<is_serializable T, is_serializable Allocator>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (std::vector<T, Allocator>)
    ;
}

#include "detail/vector.ipp"

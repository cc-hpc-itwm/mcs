// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/declare.hpp>
#include <set>

namespace mcs::serialization
{
  template< is_serializable Key
          , is_serializable Compare
          , is_serializable Allocator
          >
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (std::set<Key, Compare, Allocator>)
    ;
}

#include "detail/set.ipp"

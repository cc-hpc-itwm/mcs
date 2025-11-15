// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <map>
#include <mcs/serialization/declare.hpp>

namespace mcs::serialization
{
  template< is_serializable Key
          , is_serializable T
          , is_serializable Compare
          , is_serializable Allocator
          >
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (std::map<Key, T, Compare, Allocator>)
    ;
}

#include "detail/map.ipp"

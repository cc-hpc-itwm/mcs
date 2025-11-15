// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/declare.hpp>
#include <unordered_map>

namespace mcs::serialization
{
  template< is_serializable Key
          , is_serializable T
          , is_serializable Hash
          , is_serializable KeyEqual
          , is_serializable Allocator
          >
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (std::unordered_map<Key, T, Hash, KeyEqual, Allocator>)
    ;
}

#include "detail/unordered_map.ipp"

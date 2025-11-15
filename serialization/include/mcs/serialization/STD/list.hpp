// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <list>
#include <mcs/serialization/declare.hpp>

namespace mcs::serialization
{
  template<is_serializable T, is_serializable Allocator>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (std::list<T, Allocator>)
    ;
}

#include "detail/list.ipp"

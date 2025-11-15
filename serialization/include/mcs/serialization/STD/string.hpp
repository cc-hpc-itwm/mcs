// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/declare.hpp>
#include <string>

namespace mcs::serialization
{
  template< class CharT
          , class Traits
          , is_serializable Allocator
          >
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (std::basic_string<CharT, Traits, Allocator>)
    ;
}

#include "detail/string.ipp"

// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <string>

namespace mcs::serialization
{
  template< class CharT
          , class Traits
          , is_serializable Allocator
          >
    struct Implementation<std::basic_string<CharT, Traits, Allocator>>
  {
    using Type = std::basic_string<CharT, Traits, Allocator>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/string.ipp"

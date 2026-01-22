// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <map>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::serialization
{
  template< is_serializable Key
          , is_serializable T
          , is_serializable Compare
          , is_serializable Allocator
          >
    struct Implementation<std::map<Key, T, Compare, Allocator>>
  {
    using Type = std::map<Key, T, Compare, Allocator>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/map.ipp"

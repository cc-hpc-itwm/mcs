// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <set>

namespace mcs::serialization
{
  template< is_serializable Key
          , is_serializable Compare
          , is_serializable Allocator
          >
    struct Implementation<std::set<Key, Compare, Allocator>>
  {
    using Type = std::set<Key, Compare, Allocator>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/set.ipp"

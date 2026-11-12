// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <unordered_set>

namespace mcs::serialization
{
  template< is_serializable Key
          , is_serializable Hash
          , is_serializable KeyEqual
          , is_serializable Allocator
          >
    struct Implementation<std::unordered_set<Key, Hash, KeyEqual, Allocator>>
  {
    using Type = std::unordered_set<Key, Hash, KeyEqual, Allocator>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/unordered_set.ipp"

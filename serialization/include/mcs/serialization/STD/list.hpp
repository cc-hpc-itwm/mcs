// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <list>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::serialization
{
  template<is_serializable T, is_serializable Allocator>
    struct Implementation<std::list<T, Allocator>>
  {
    using Type = std::list<T, Allocator>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/list.ipp"

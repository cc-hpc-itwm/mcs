// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <vector>

namespace mcs::serialization
{
  template<is_serializable T, is_serializable Allocator>
    struct Implementation<std::vector<T, Allocator>>
  {
    using Type = std::vector<T, Allocator>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/vector.ipp"

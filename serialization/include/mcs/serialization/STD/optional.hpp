// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <optional>

namespace mcs::serialization
{
  template<is_serializable T>
    struct Implementation<std::optional<T>>
  {
    using Type = std::optional<T>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/optional.ipp"

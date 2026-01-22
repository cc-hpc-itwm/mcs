// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <tuple>

namespace mcs::serialization
{
  template<is_serializable... Ts>
    struct Implementation<std::tuple<Ts...>>
  {
    using Type = std::tuple<Ts...>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/tuple.ipp"

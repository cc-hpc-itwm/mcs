// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>

namespace mcs::serialization
{
  template<is_serializable T>
    constexpr auto load (IArchive&) -> T;
}

#include "detail/load.ipp"

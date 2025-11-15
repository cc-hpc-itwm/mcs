// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::serialization
{
  // throws if buffer contains more than a T
  template<is_serializable T, typename... BufferArgs>
    auto load_from (BufferArgs&&...) -> T;
  template<is_serializable T>
    auto load_from (std::filesystem::path) -> T;
}

#include "detail/load_from.ipp"

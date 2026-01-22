// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::serialization
{
  template<>
    struct Implementation<std::filesystem::path>
  {
    using Type = std::filesystem::path;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

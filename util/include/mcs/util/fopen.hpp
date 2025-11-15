// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/util/OpenFile.hpp>

namespace mcs::util
{
  [[nodiscard]] auto fopen
    ( std::filesystem::path
    , char const* mode
    ) -> OpenFile
    ;
}

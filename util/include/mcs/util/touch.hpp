// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>

namespace mcs::util
{
  auto touch (std::filesystem::path) -> std::filesystem::path;
}

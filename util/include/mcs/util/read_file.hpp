// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <string>

namespace mcs::util
{
  [[nodiscard]] auto read_file (std::filesystem::path const&) -> std::string;
}

// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/fopen.hpp>
#include <mcs/util/touch.hpp>
#include <utility>

namespace mcs::util
{
  auto touch (std::filesystem::path path) -> std::filesystem::path
  {
    std::ignore = fopen (path, "ab");

    return path;
  }
}

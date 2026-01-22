// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <optional>

namespace mcs::fuse::content::result::write
{
  struct Success
  {
    std::optional<size_t> new_size{};
  };
}

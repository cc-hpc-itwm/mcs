// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/fuse/content/implementation/storage/Part.hpp>
#include <type_traits>

namespace mcs::fuse::content::storage
{
  // Retrieve and use parts of a file.
  //
  template<typename UsePart>
    requires (std::is_invocable_v<UsePart, Part const&>)
    auto for_each_part (std::filesystem::path, UsePart);
}

#include "detail/for_each_part.ipp"

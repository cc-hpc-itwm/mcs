// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/fuse/low_level.hpp>
#include <string>
#include <vector>

namespace mcs::fuse::state
{
  // Snapshot taken by opendir and consumed by readdir and readdirplus.
  //
  struct DirectoryHandle
  {
    struct Entry
    {
      std::string name;
      ::fuse_entry_param entry_param;
    };
    std::vector<Entry> entries;
  };
}

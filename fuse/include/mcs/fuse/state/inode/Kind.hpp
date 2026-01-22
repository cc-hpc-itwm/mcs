// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/state/inode/kind/Directory.hpp>
#include <mcs/fuse/state/inode/kind/File.hpp>
#include <mcs/fuse/state/inode/kind/Symlink.hpp>

namespace mcs::fuse::state::inode
{
  template<typename Content, typename Kind>
    concept is_kind =  std::is_same_v<Kind, kind::Directory>
                    || std::is_same_v<Kind, kind::File<Content>>
                    || std::is_same_v<Kind, kind::Symlink>
    ;
}

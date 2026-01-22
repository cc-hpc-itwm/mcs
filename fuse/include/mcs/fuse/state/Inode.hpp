// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/Content.hpp>
#include <mcs/fuse/state/inode/Kind.hpp>

namespace mcs::fuse::state
{
  template<is_content Content, typename Kind>
    requires (inode::is_kind<Content, Kind>)
    struct Inode;
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/Content.hpp>
#include <mcs/fuse/state/Access.hpp>
#include <mcs/util/not_null.hpp>

namespace mcs::fuse::state::inode::kind
{
  template<is_content Content>
    struct File
  {
    mcs::util::not_null<typename Content::State> _content_state;
    mcs::util::not_null<access::Write const> _write_access;
  };
}

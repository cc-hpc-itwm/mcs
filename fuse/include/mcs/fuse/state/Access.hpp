// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/util/Lock.hpp>
#include <mcs/util/lock/queue/Fast.hpp>

namespace mcs::fuse::state::access
{
  using Read = util::Lock<util::lock::mode::Shared, util::lock::queue::Fast>;
  using Write = util::Lock<util::lock::mode::Unique, util::lock::queue::Fast>;
}

namespace mcs::fuse::state
{
  template<typename Access>
    concept writing = std::is_same_v<Access, access::Write>;

  template<typename Access>
    concept reading = writing<Access> || std::is_same_v<Access, access::Read>;
}

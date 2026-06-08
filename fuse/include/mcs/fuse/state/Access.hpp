// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/util/concurrency/SharedMutex.hpp>
#include <mcs/util/concurrency/queue/Fast.hpp>
#include <mutex>
#include <shared_mutex>

namespace mcs::fuse::state::access
{
  using Mutex
    = util::concurrency::SharedMutex<util::concurrency::queue::Fast>
    ;
  using Read = std::shared_lock<Mutex>;
  using Write = std::unique_lock<Mutex>;
}

namespace mcs::fuse::state
{
  template<typename Access>
    concept writing = std::is_same_v<Access, access::Write>;

  template<typename Access>
    concept reading = writing<Access> || std::is_same_v<Access, access::Read>;
}

// Copyright (C) 2022,2024-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/ScopedRunningIOContext.hpp>

namespace mcs::rpc
{
  auto ScopedRunningIOContext::join() noexcept -> void
  {
    ScopedIOContext::join();
  }
  ScopedRunningIOContext::~ScopedRunningIOContext() = default;
}

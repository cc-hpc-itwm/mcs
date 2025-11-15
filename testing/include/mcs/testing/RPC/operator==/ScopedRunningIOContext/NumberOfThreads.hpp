// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/ScopedRunningIOContext.hpp>

namespace mcs::rpc
{
  constexpr auto operator==
    ( ScopedRunningIOContext::NumberOfThreads const& lhs
    , ScopedRunningIOContext::NumberOfThreads const& rhs
    ) noexcept -> bool
  {
    return lhs.value == rhs.value;
  }
}

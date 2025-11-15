// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/multi_client/ParallelCallsLimit.hpp>

namespace mcs::rpc::multi_client
{
  ParallelCalls::AtMost::Error::MustBePositive::MustBePositive
    (
    ) noexcept
      : mcs::Error {"Maximum number of parallel calls must be positive."}
  {}
  ParallelCalls::AtMost::Error::MustBePositive::~MustBePositive() = default;
}

// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/ScopedRunningIOContext.hpp>

namespace mcs::rpc
{
  auto ScopedRunningIOContext::join() noexcept -> void
  {
    _threads.join();
  }
  ScopedRunningIOContext::~ScopedRunningIOContext()
  {
    asio::io_context::stop();
    join();
  }

  ScopedRunningIOContext::NumberOfThreads::Error::MustBePositive::MustBePositive
    (
    ) noexcept
      : mcs::Error {"Number of threads must be positive."}
  {}
  ScopedRunningIOContext::NumberOfThreads::Error::MustBePositive::~MustBePositive() = default;
}

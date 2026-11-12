// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/ScopedIOContext.hpp>

namespace mcs::rpc
{
  // io_context that stops when one of the specified signals arrives
  // uses a thread pool for multi-threaded execution. At creation the
  // io_context's run() method is called in each thread of the
  // pool. Upon destruction the io_context's stop() method is called
  // and the threads are joined.
  //
  // EXAMPLE:
  //   auto io_context
  //     { ScopedRunningIOContext
  //         {ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM}
  //     };
  //
  struct [[nodiscard]] ScopedRunningIOContext : public ScopedIOContext
  {
    using NumberOfThreads = ScopedIOContext::NumberOfThreads;

    template<typename... Signals>
      explicit ScopedRunningIOContext
        ( NumberOfThreads
        , Signals&&...
        );

    // For servers that want to join without calling stop()
    // explicitly.
    //
    auto join() noexcept -> void;

    ~ScopedRunningIOContext();
    ScopedRunningIOContext (ScopedRunningIOContext const&) = delete;
    ScopedRunningIOContext (ScopedRunningIOContext&&) = delete;
    auto operator= (ScopedRunningIOContext const&) -> ScopedRunningIOContext& = delete;
    auto operator= (ScopedRunningIOContext&&) -> ScopedRunningIOContext& = delete;
  };
}

#include "detail/ScopedRunningIOContext.ipp"

// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::rpc
{
  template<typename... Signals>
    ScopedRunningIOContext::ScopedRunningIOContext
      ( NumberOfThreads number_of_threads
      , Signals&&... signals
      )
        : ScopedIOContext
          { number_of_threads
          , std::forward<Signals> (signals)...
          }
  {
    start();
  }
}

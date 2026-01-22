// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/lock/Concepts.hpp>
#include <mutex>

namespace mcs::util::lock::queue
{
  // The Fast queue does not guarantee to serve in strict FIFO
  // order. The Fast queue is "as fast" as the std::mutex
  // implementation, which is good enough for most use cases.
  //
  struct Fast
  {
    auto wait_to_be_served() -> void;
    auto next_in_queue() -> void;

  private:
    std::mutex _guard;
  };
  static_assert (is_request_queue<Fast>);
}

// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <atomic>
#include <cstdint>
#include <mcs/util/concurrency/Concepts.hpp>

namespace mcs::util::concurrency::queue
{
  // The FIFO queue serves requests strictly in FIFO order. Later
  // requests never overtake earlier requests.
  //
  struct FIFO
  {
    auto wait_to_be_served() -> void;
    auto next_in_queue() -> void;

  private:
    // The queue of lock requests is managed by _next_slot_to_serve
    // and _next_available_slot. It holds ! (_next_available_slot <
    // _next_slot_to_serve). Requesters get a slot when they arrive
    // and are served if their slot equals the next slot to be served.
    //
    using Ticket = std::uint_fast64_t;
    std::atomic<Ticket> _next_slot_to_serve {Ticket {0}};
    std::atomic<Ticket> _next_available_slot {Ticket {0}};
  };
  static_assert (is_request_queue<FIFO>);
}

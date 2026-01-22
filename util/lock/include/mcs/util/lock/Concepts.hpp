// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <type_traits>

namespace mcs::util::lock::mode
{
  struct Shared{};
  struct Unique{};
}

namespace mcs::util::lock
{
  template<typename A>
    concept is_mode =  std::is_same_v<A, mode::Shared>
                    || std::is_same_v<A, mode::Unique>
      ;
}

namespace mcs::util::lock
{
  // The requests to get the lock are managed in a queue like
  // structure that only ever serves a single entry.
  //
  template<typename RequestQueue>
    concept is_request_queue = requires
        ( RequestQueue& queue
        )
      {
        // Blocks until the caller is served.
        //
        { queue.wait_to_be_served()
        } -> std::convertible_to<void>
        ;
        // Tell the queue to proceed with the next entry in the queue.
        //
        { queue.next_in_queue()
        } -> std::convertible_to<void>
        ;
      }
    ;
}

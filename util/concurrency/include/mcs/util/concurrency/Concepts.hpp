// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mutex>
#include <shared_mutex>
#include <type_traits>

namespace mcs::util::concurrency
{
  // Implements the C++ named requirement BasicLockable.
  //
  template<typename Mutex>
    concept has_exclusive_locking
      = requires (Mutex m)
        {
          { m.lock()
          } -> std::same_as<void>
          ;
          { m.unlock()
          } -> std::same_as<void>
          ;
        };

  // Implements the shared portion of the C++ named requirement
  // SharedLockable. Note: the standard has no BasicSharedLockable.
  //
  template<typename Mutex>
    concept has_shared_locking
      = requires (Mutex m)
        {
          { m.lock_shared()
          } -> std::same_as<void>
          ;
          { m.unlock_shared()
          } -> std::same_as<void>
          ;
        };
}

namespace mcs::util::concurrency
{
  template<typename Lock>
    concept is_unique_lock
      =  std::same_as<std::unique_lock<typename Lock::mutex_type>, Lock>
      && has_exclusive_locking<typename Lock::mutex_type>
      ;

  template<typename Lock>
    concept is_shared_lock
      =  std::same_as<std::shared_lock<typename Lock::mutex_type>, Lock>
      && has_shared_locking<typename Lock::mutex_type>
      ;

  template<typename Lock>
    concept is_lock
      =  is_unique_lock<Lock>
      || is_shared_lock<Lock>
      ;
}

namespace mcs::util::concurrency
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

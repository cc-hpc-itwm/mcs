// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <atomic>
#include <cstdint>
#include <mcs/util/lock/Concepts.hpp>
#include <mcs/util/not_null.hpp>
#include <type_traits>

namespace mcs::util
{
  template<lock::is_mode, lock::is_request_queue> struct Lock;
}

namespace mcs::util::lock
{
  // A shared mutex that allows for (fair) acquisition of locks.
  //
  // - Multiple shared locks can exist at the same time.
  // - At most one unique lock can exist at the same time.
  // - If a unique lock exists, then no shared lock exists.
  // - If a shared lock exists, then no unique lock exists.
  //
  // Depending on the properties of the request queue, the acquisition
  // is fair or not: If the request queue guarantees strict FIFO
  // order, then the acquisition is fair, e.g. no requester will
  // starve for ever.
  //
  template<is_request_queue RequestQueue>
    struct SharedMutex
  {
  private:
    template<is_mode, is_request_queue> friend struct util::Lock;

    mutable RequestQueue _queue;

    // The number of active readers is used by a served writer to wait
    // for all of them to finish before entering the critical section.
    //
    using NumberOfReaders = std::uint_fast64_t;
    mutable std::atomic<NumberOfReaders> _active_readers {NumberOfReaders {0}};
  };
}

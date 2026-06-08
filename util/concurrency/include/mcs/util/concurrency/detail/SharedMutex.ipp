// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/concurrency/detail/wait_for_value.hpp>

namespace mcs::util::concurrency
{
  template<is_request_queue RequestQueue>
    auto SharedMutex<RequestQueue>::lock() -> void
    {
      _queue.wait_to_be_served();

      concurrency::detail::wait_for_value
        ( _active_readers, NumberOfReaders {0}
        );
    }

  template<is_request_queue RequestQueue>
    auto SharedMutex<RequestQueue>::unlock() -> void
    {
      // Writers allow the next requester to proceed only after they
      // have left the critical section.
      //
      _queue.next_in_queue();
    }

  template<is_request_queue RequestQueue>
    auto SharedMutex<RequestQueue>::lock_shared() -> void
    {
      _queue.wait_to_be_served();

      _active_readers.fetch_add (NumberOfReaders {1});

      // Readers allow the next requester to proceed while they are in
      // the critical section. More readers can enter the critical
      // section. Writers will wait until all readers have left the
      // critical section.
      //
      _queue.next_in_queue();
    }

  template<is_request_queue RequestQueue>
    auto SharedMutex<RequestQueue>::unlock_shared() -> void
    {
      _active_readers.fetch_sub (NumberOfReaders {1});

      // \note There is at most one writer waiting, so notify_one is
      // sufficient.
      _active_readers.notify_one();
    }
}

// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/lock/detail/wait_for_value.hpp>
#include <memory>

namespace mcs::util
{
  template<lock::is_request_queue RequestQueue>
    Lock<lock::mode::Unique, RequestQueue>::Lock
      ( util::not_null<lock::SharedMutex<RequestQueue>> guard
      )
        : _guard {guard}
  {
    _guard->_queue.wait_to_be_served();

    lock::detail::wait_for_value
      ( _guard->_active_readers
      , typename lock::SharedMutex<RequestQueue>::NumberOfReaders {0}
      );
  }

  template<lock::is_request_queue RequestQueue>
    Lock<lock::mode::Unique, RequestQueue>::~Lock()
  {
    // Writers allow the next requester to proceed only after they
    // have left the critical section.
    //
    _guard->_queue.next_in_queue();
  }
}

namespace mcs::util
{
  template<lock::is_request_queue RequestQueue>
    Lock<lock::mode::Shared, RequestQueue>::Lock
      ( util::not_null<lock::SharedMutex<RequestQueue> const> guard
      )
        : _guard {guard}
  {
    _guard->_queue.wait_to_be_served();

    _guard->_active_readers.fetch_add
      ( typename lock::SharedMutex<RequestQueue>::NumberOfReaders {1}
      );

    // Readers allow the next requester to proceed while they are in
    // the critical section. More readers can enter the critical
    // section. Writers will wait until all readers have left the
    // critical section.
    //
    _guard->_queue.next_in_queue();
  }

  template<lock::is_request_queue RequestQueue>
    Lock<lock::mode::Shared, RequestQueue>::~Lock()
  {
    _guard->_active_readers.fetch_sub
      ( typename lock::SharedMutex<RequestQueue>::NumberOfReaders {1}
      );
    // \note There is at most one writer waiting, so notify_one is
    // sufficient.
    _guard->_active_readers.notify_one();
  }
}

namespace mcs::util
{
  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto unique_lock
      ( not_null<lock::SharedMutex<RequestQueue>> guard
      )
  {
    return Lock<lock::mode::Unique, RequestQueue> {guard};
  }

  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto unique_lock
      ( lock::SharedMutex<RequestQueue>* guard
      )
  {
    return Lock<lock::mode::Unique, RequestQueue> {not_null {guard}};
  }

  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto unique_lock
      ( lock::SharedMutex<RequestQueue>& guard
      )
  {
    return Lock<lock::mode::Unique, RequestQueue> {std::addressof (guard)};
  }
}

namespace mcs::util
{
  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto shared_lock
      ( not_null<lock::SharedMutex<RequestQueue> const> guard
      )
  {
    return Lock<lock::mode::Shared, RequestQueue> {guard};
  }

  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto shared_lock
      ( lock::SharedMutex<RequestQueue> const* guard
      )
  {
    return Lock<lock::mode::Shared, RequestQueue> {not_null {guard}};
  }

  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto shared_lock
      ( lock::SharedMutex<RequestQueue> const& guard
      )
  {
    return Lock<lock::mode::Shared, RequestQueue> {std::addressof (guard)};
  }
}

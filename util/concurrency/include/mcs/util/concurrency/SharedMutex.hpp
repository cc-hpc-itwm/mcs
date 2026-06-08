// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <atomic>
#include <cstdint>
#include <mcs/util/concurrency/Concepts.hpp>
#include <mcs/util/concurrency/queue/FIFO.hpp>
#include <mcs/util/concurrency/queue/Fast.hpp>
#include <type_traits>

namespace mcs::util::concurrency
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
  //
  // EXAMPLE:
  //
  // Wrap an UnsynchronizedContainer into a SynchronizedContainer with
  // shared read access and unique write access.
  //
  //    struct UnsynchronizedContainer
  //    {
  //      auto cm() const -> void;
  //      auto mm() -> void;
  //    };
  //
  //    struct SynchronizedContainer : private UnsynchronizedContainer
  //    {
  //      using UnsynchronizedContainer::UnsynchronizedContainer;
  //      using Mutex
  //        = mcs::util::concurrency::SharedMutex
  //            <mcs::util::concurrency::queue::Fast>
  //        ;
  //
  //      struct ReadAccess
  //      {
  //        auto cm() const
  //        {
  //          return _unsynchronized_container->cm();
  //        }
  //
  //      private:
  //        friend struct SynchronizedContainer;
  //
  //        std::shared_lock<Mutex> _lock;
  //        UnsynchronizedContainer const* _unsynchronized_container;
  //
  //        ReadAccess
  //          ( UnsynchronizedContainer const* unsynchronized_container
  //          , Mutex* mutex
  //          )
  //          : _lock {*mutex}
  //          , _unsynchronized_container {unsynchronized_container}
  //        {}
  //      };
  //
  //      struct WriteAccess
  //      {
  //        auto mm() const
  //        {
  //          return _unsynchronized_container->mm();
  //        }
  //
  //      private:
  //        friend struct SynchronizedContainer;
  //
  //        std::unique_lock<Mutex> _lock;
  //        UnsynchronizedContainer* _unsynchronized_container;
  //
  //        WriteAccess
  //          ( UnsynchronizedContainer* unsynchronized_container
  //          , Mutex* mutex
  //          )
  //          : _lock {*mutex}
  //          , _unsynchronized_container {unsynchronized_container}
  //        {}
  //      };
  //
  //      [[nodiscard]] auto read_access() const -> ReadAccess
  //      {
  //        return ReadAccess {this, std::addressof (_guard)};
  //      }
  //      [[nodiscard]] auto write_access() -> WriteAccess
  //      {
  //        return WriteAccess {this, std::addressof (_guard)};
  //      }
  //
  //    private:
  //      mutable Mutex _guard;
  //    };
  //
  //
  // EXAMPLE:
  //
  // Wrap an UnsynchronizedContainer into a SynchronizedContainer with
  // shared read access and unique read&write access.
  // QueueType is parameterized.
  //
  //    struct UnsynchronizedContainer
  //    {
  //      auto cm() const -> void;
  //      auto mm() -> void;
  //    };
  //
  //    template<concurrency::is_request_queue QueueType>
  //      struct SynchronizedContainer : private UnsynchronizedContainer
  //    {
  //      using UnsynchronizedContainer::UnsynchronizedContainer;
  //      using Mutex = concurrency::SharedMutex<QueueType>;
  //
  //      template<concurrency::is_lock Lock>
  //        struct Access
  //      {
  //        auto cm() const
  //        {
  //          return _const_unsynchronized_container->cm();
  //        }
  //
  //      protected:
  //        Lock _lock;
  //        UnsynchronizedContainer const* _const_unsynchronized_container;
  //
  //        friend struct SynchronizedContainer;
  //
  //        Access
  //          ( UnsynchronizedContainer const* unsynchronized_container
  //          , Mutex* mutex
  //          )
  //          : _lock {*mutex}
  //          , _const_unsynchronized_container {unsynchronized_container}
  //        {}
  //      };
  //      using ReadAccess = Access<std::shared_lock<Mutex>>;
  //
  //      struct ReadWriteAccess : public Access<std::unique_lock<Mutex>>
  //      {
  //        auto mm()
  //        {
  //          return _mutable_unsynchronized_container->mm();
  //        }
  //
  //      private:
  //        friend struct SynchronizedContainer;
  //        UnsynchronizedContainer* const _mutable_unsynchronized_container;
  //
  //        ReadWriteAccess
  //          ( UnsynchronizedContainer* unsynchronized_container
  //          , Mutex* mutex
  //          )
  //          : Access<std::unique_lock<Mutex>>
  //              {unsynchronized_container, mutex}
  //          , _mutable_unsynchronized_container {unsynchronized_container}
  //        {}
  //      };
  //
  //      [[nodiscard]] auto read_access() const -> ReadAccess
  //      {
  //        return ReadAccess {this, std::addressof (_guard)};
  //      }
  //      [[nodiscard]] auto read_write_access() -> ReadWriteAccess
  //      {
  //        return ReadWriteAccess {this, std::addressof (_guard)};
  //      }
  //    private:
  //      mutable Mutex _guard;
  //    };
  //
  template<is_request_queue RequestQueue>
    struct SharedMutex
  {
    auto lock() -> void;
    auto unlock() -> void;

    auto lock_shared() -> void;
    auto unlock_shared() -> void;

  private:
    RequestQueue _queue;

    // The number of active readers is used by a served writer to wait
    // for all of them to finish before entering the critical section.
    //
    using NumberOfReaders = std::uint_fast64_t;
    std::atomic<NumberOfReaders> _active_readers {NumberOfReaders {0}};
  };

  static_assert (has_exclusive_locking<SharedMutex<queue::Fast>>);
  static_assert (has_exclusive_locking<SharedMutex<queue::FIFO>>);
  static_assert (has_shared_locking<SharedMutex<queue::Fast>>);
  static_assert (has_shared_locking<SharedMutex<queue::FIFO>>);
}

#include "detail/SharedMutex.ipp"

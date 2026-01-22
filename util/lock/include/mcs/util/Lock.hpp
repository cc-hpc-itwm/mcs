// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/lock/Concepts.hpp>
#include <mcs/util/lock/SharedMutex.hpp>
#include <mcs/util/not_null.hpp>

namespace mcs::util
{
  // Fair shared read and unique read&write locks with support for
  // single-lock bulk operations.
  //
  // EXAMPLE:
  //
  // Wrap an UnsynchronizedContainer into a SynchronizedContainer with
  // shared read access and unique write access.
  //
  //     using LockQueueType = lock::queue::FIFO;
  //
  //     struct UnsynchronizedContainer
  //     {
  //       auto cm() const -> void;
  //       auto mm() -> void;
  //     };
  //
  //     struct SynchronizedContainer : private UnsynchronizedContainer
  //     {
  //       using UnsynchronizedContainer::UnsynchronizedContainer;
  //
  //       struct ReadAccess
  //       {
  //         auto cm() const
  //         {
  //           return _unsynchronized_container->cm();
  //         }
  //
  //       private:
  //         friend struct SynchronizedContainer;
  //         Lock<lock::mode::Shared, LockQueueType> _lock;
  //         UnsynchronizedContainer const* _unsynchronized_container;
  //         template<typename... LockArgs>
  //           ReadAccess
  //             ( UnsynchronizedContainer const* unsynchronized_container
  //             , LockArgs&&... lock_args
  //             )
  //             : _lock {std::forward<LockArgs> (lock_args)...}
  //             , _unsynchronized_container {unsynchronized_container}
  //         {}
  //       };
  //
  //       struct WriteAccess
  //       {
  //         auto mm() const
  //         {
  //           return _unsynchronized_container->mm();
  //         }
  //
  //       private:
  //         friend struct SynchronizedContainer;
  //         Lock<lock::mode::Unique, LockQueueType> _lock;
  //         UnsynchronizedContainer* _unsynchronized_container;
  //         template<typename... LockArgs>
  //           WriteAccess
  //             ( UnsynchronizedContainer* unsynchronized_container
  //             , LockArgs&&... lock_args
  //             )
  //             : _lock {std::forward<LockArgs> (lock_args)...}
  //             , _unsynchronized_container {unsynchronized_container}
  //         {}
  //       };
  //
  //       [[nodiscard]] auto read_access() const -> ReadAccess
  //       {
  //         return ReadAccess {this, std::addressof (_guard)};
  //       }
  //       [[nodiscard]] auto write_access() -> WriteAccess
  //       {
  //         return WriteAccess {this, std::addressof (_guard)};
  //       }
  //
  //     private:
  //       lock::SharedMutex<LockQueueType> _guard;
  //     };
  //
  // EXAMPLE:
  //
  // Wrap an UnsynchronizedContainer into a SynchronizedContainer with
  // shared read access and unique read&write access.
  //
  //     using LockQueueType = lock::queue::Fast;
  //
  //     struct UnsynchronizedContainer
  //     {
  //       auto cm() const -> void;
  //       auto mm() -> void;
  //     };
  //
  //     struct SynchronizedContainer : private UnsynchronizedContainer
  //     {
  //       using UnsynchronizedContainer::UnsynchronizedContainer;
  //
  //       template<lock::is_mode Mode>
  //         struct Access
  //       {
  //         auto cm() const
  //         {
  //           return _unsynchronized_container->cm();
  //         }
  //
  //       private:
  //         friend struct SynchronizedContainer;
  //
  //         Lock<Mode, LockQueueType> _lock;
  //         UnsynchronizedContainer const* _unsynchronized_container;
  //
  //         template<typename... LockArgs>
  //           Access
  //             ( UnsynchronizedContainer const* unsynchronized_container
  //             , LockArgs&&... lock_args
  //             )
  //             : _lock {std::forward<LockArgs> (lock_args)...}
  //             , _unsynchronized_container {unsynchronized_container}
  //         {}
  //       };
  //       using ReadAccess = Access<lock::mode::Shared>;
  //
  //       struct ReadWriteAccess : public Access<lock::mode::Unique>
  //       {
  //         using Access<lock::mode::Unique>::cm;
  //         auto mm() const
  //         {
  //           return _unsynchronized_container->mm();
  //         }
  //
  //       private:
  //         friend struct SynchronizedContainer;
  //         UnsynchronizedContainer* _unsynchronized_container;
  //         template<typename... LockArgs>
  //           ReadWriteAccess
  //             ( UnsynchronizedContainer* unsynchronized_container
  //             , LockArgs&&... lock_args
  //             )
  //             : Access<lock::mode::Unique>
  //               { unsynchronized_container
  //               , std::forward<LockArgs> (lock_args)...
  //               }
  //         {}
  //       };
  //
  //       [[nodiscard]] auto read_access() const -> ReadAccess
  //       {
  //         return ReadAccess {this, std::addressof (_guard)};
  //       }
  //       [[nodiscard]] auto write_access() -> ReadWriteAccess
  //       {
  //         return ReadWriteAccess {this, std::addressof (_guard)};
  //       }
  //     private:
  //       lock::SharedMutex<LockQueueType> _guard;
  //     };
  template<lock::is_mode, lock::is_request_queue> struct Lock;
}

namespace mcs::util
{
  template<lock::is_request_queue RequestQueue>
    struct Lock<lock::mode::Unique, RequestQueue>
  {
    [[nodiscard]] explicit Lock (util::not_null<lock::SharedMutex<RequestQueue>>);

    ~Lock();
    Lock (Lock const&) = delete;
    Lock (Lock&&) = delete;
    auto operator= (Lock const&) -> Lock& = delete;
    auto operator= (Lock&&) -> Lock& = delete;

  private:
    util::not_null<lock::SharedMutex<RequestQueue>> _guard;
  };
}

namespace mcs::util
{
  template<lock::is_request_queue RequestQueue>
    struct Lock<lock::mode::Shared, RequestQueue>
  {
    [[nodiscard]] explicit Lock
      ( util::not_null<lock::SharedMutex<RequestQueue> const>
      );

    ~Lock();
    Lock (Lock const&) = delete;
    Lock (Lock&&) = delete;
    auto operator= (Lock const&) -> Lock& = delete;
    auto operator= (Lock&&) -> Lock& = delete;

  private:
    util::not_null<lock::SharedMutex<RequestQueue> const> _guard;
  };
}

namespace mcs::util
{
  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto unique_lock (not_null<lock::SharedMutex<RequestQueue>>);

  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto unique_lock (lock::SharedMutex<RequestQueue>*);

  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto unique_lock (lock::SharedMutex<RequestQueue>&);
}

namespace mcs::util
{
  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto shared_lock (not_null<lock::SharedMutex<RequestQueue> const>);

  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto shared_lock (lock::SharedMutex<RequestQueue> const*);

  template<lock::is_request_queue RequestQueue>
    [[nodiscard]] auto shared_lock (lock::SharedMutex<RequestQueue> const&);
}

#include "detail/Lock.ipp"

// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <atomic>
#include <barrier>
#include <chrono>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <iterator>
#include <latch>
#include <list>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/concurrency/Concepts.hpp>
#include <mcs/util/concurrency/SharedMutex.hpp>
#include <mcs/util/concurrency/queue/FIFO.hpp>
#include <mcs/util/concurrency/queue/Fast.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace mcs::util
{
  namespace
  {
    using Queues = ::testing::Types
      < concurrency::queue::FIFO
      , concurrency::queue::Fast
      >;

    template<class> struct MCSLockR : public testing::random::Test{};
    TYPED_TEST_SUITE (MCSLockR, Queues);
  }

  TYPED_TEST (MCSLockR, lock_write_access_example_from_documentation_compiles)
  {
    // EXAMPLE:
    //
    // Wrap an UnsynchronizedContainer into a SynchronizedContainer with
    // shared read access and unique write access.

    struct UnsynchronizedContainer
    {
      auto cm() const -> void;
      auto mm() -> void;
    };

    struct SynchronizedContainer : private UnsynchronizedContainer
    {
      using UnsynchronizedContainer::UnsynchronizedContainer;
      using Mutex = mcs::util::concurrency::SharedMutex<TypeParam>;

      struct ReadAccess
      {
        auto cm() const
        {
          return _unsynchronized_container->cm();
        }

      private:
        friend struct SynchronizedContainer;

        std::shared_lock<Mutex> _lock;
        UnsynchronizedContainer const* _unsynchronized_container;

        ReadAccess
          ( UnsynchronizedContainer const* unsynchronized_container
          , Mutex* mutex
          )
          : _lock {*mutex}
          , _unsynchronized_container {unsynchronized_container}
        {}
      };

      struct WriteAccess
      {
        auto mm() const
        {
          return _unsynchronized_container->mm();
        }

      private:
        friend struct SynchronizedContainer;

        std::unique_lock<Mutex> _lock;
        UnsynchronizedContainer* _unsynchronized_container;

        WriteAccess
          ( UnsynchronizedContainer* unsynchronized_container
          , Mutex* mutex
          )
          : _lock {*mutex}
          , _unsynchronized_container {unsynchronized_container}
        {}
      };

      [[nodiscard]] auto read_access() const -> ReadAccess
      {
        return ReadAccess {this, std::addressof (_guard)};
      }
      [[nodiscard]] auto write_access() -> WriteAccess
      {
        return WriteAccess {this, std::addressof (_guard)};
      }

    private:
      mutable Mutex _guard;
    };
  }

  namespace
  {
    // EXAMPLE:
    //
    // Wrap an UnsynchronizedContainer into a SynchronizedContainer with
    // shared read access and unique read&write access.
    // QueueType is parameterized.

    struct UnsynchronizedContainer
    {
      auto cm() const -> void;
      auto mm() -> void;
    };

    template<concurrency::is_request_queue QueueType>
      struct SynchronizedContainer : private UnsynchronizedContainer
    {
      using UnsynchronizedContainer::UnsynchronizedContainer;
      using Mutex = concurrency::SharedMutex<QueueType>;

      template<concurrency::is_lock Lock>
        struct Access
      {
        auto cm() const
        {
          return _unsynchronized_container->cm();
        }

      protected:
        Lock _lock;

      private:
        UnsynchronizedContainer const* _unsynchronized_container;

        friend struct SynchronizedContainer;

        Access
          ( UnsynchronizedContainer const* unsynchronized_container
          , Mutex* mutex
          )
          : _lock {*mutex}
          , _unsynchronized_container {unsynchronized_container}
        {}
      };
      using ReadAccess = Access<std::shared_lock<Mutex>>;

      struct ReadWriteAccess : public Access<std::unique_lock<Mutex>>
      {
        auto mm()
        {
          return _mutable_unsynchronized_container->mm();
        }

      private:
        friend struct SynchronizedContainer;
        UnsynchronizedContainer * const _mutable_unsynchronized_container;

        ReadWriteAccess
          ( UnsynchronizedContainer* unsynchronized_container
          , Mutex* mutex
          )
          : Access<std::unique_lock<Mutex>>
              {unsynchronized_container, mutex}
          , _mutable_unsynchronized_container {unsynchronized_container}
        {}
      };

      [[nodiscard]] auto read_access() const -> ReadAccess
      {
        return ReadAccess {this, std::addressof (_guard)};
      }
      [[nodiscard]] auto read_write_access() -> ReadWriteAccess
      {
        return ReadWriteAccess {this, std::addressof (_guard)};
      }
    private:
      mutable Mutex _guard;
    };
  }

  TYPED_TEST (MCSLockR, lock_read_write_access_example_from_documentation_compiles)
  {
    using QueueType = TypeParam;
    [[maybe_unused]] SynchronizedContainer<QueueType> c;
  }

  TYPED_TEST (MCSLockR, a_guard_can_have_multiple_shared_locks_at_the_same_time)
  {
    auto guard {concurrency::SharedMutex<TypeParam>{}};

    auto const number_of_readers
      { std::invoke
        ( testing::random::value<int>
          { testing::random::value<int>::Min {1}
          , testing::random::value<int>::Max {1000}
          }
        )
      };

    auto in_critical_section {std::atomic<int> {0}};
    auto barrier {std::barrier {number_of_readers}};

    auto readers {std::list<std::future<bool>>{}};

    std::generate_n
      ( std::inserter (readers, std::end (readers))
      , number_of_readers
      , [&]
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                {
                  auto const lock {std::shared_lock (guard)};

                  in_critical_section.fetch_add (1);

                  barrier.arrive_and_wait();
                }
                return in_critical_section == number_of_readers;
              }
            );
        }
      );

    ASSERT_TRUE
      ( std::ranges::all_of
        ( readers
        , [] (auto& reader)
          {
            return reader.get();
          }
        )
      );
  }

  TYPED_TEST (MCSLockR, a_guard_can_have_at_most_one_unique_lock)
  {
    auto guard {concurrency::SharedMutex<TypeParam>{}};

    auto const number_of_writers
      { std::invoke
        ( testing::random::value<int>
          { testing::random::value<int>::Min {1}
          , testing::random::value<int>::Max {1000}
          }
        )
      };

    auto in_critical_section {std::atomic<int> {0}};

    auto writers {std::list<std::future<bool>>{}};

    std::generate_n
      ( std::inserter (writers, std::end (writers))
      , number_of_writers
      , [&]
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                {
                  auto const lock {std::unique_lock (guard)};

                  in_critical_section.fetch_add (1);

                  if (in_critical_section.load() != 1)
                  {
                    return false;
                  };

                  in_critical_section.fetch_sub (1);
                }

                return true;
              }
            );
        }
      );

    ASSERT_TRUE
      ( std::ranges::all_of
        ( writers
        , [] (auto& writer)
          {
            return writer.get();
          }
        )
      );
  }

  TYPED_TEST (MCSLockR, writer_waits_for_all_ongoing_readers_to_finish)
  {
    auto guard {concurrency::SharedMutex<TypeParam>{}};

    auto const number_of_readers
      { std::invoke
        ( testing::random::value<int>
          { testing::random::value<int>::Min {1}
          , testing::random::value<int>::Max {1000}
          }
        )
      };

    auto readers_done {std::atomic<int>{}};
    auto readers {std::list<std::future<void>>{}};

    auto readers_started {std::latch {number_of_readers}};

    std::generate_n
      ( std::inserter (readers, std::end (readers))
      , number_of_readers
      , [&]
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                auto const lock {std::shared_lock (guard)};

                readers_started.count_down();

                std::this_thread::sleep_for
                  ( std::chrono::milliseconds
                    ( std::invoke
                      ( testing::random::value<int>
                        { testing::random::value<int>::Min {100}
                        , testing::random::value<int>::Max {200}
                        }
                      )
                    )
                  );

                readers_done.fetch_add (1);
              }
            );
        }
      );


    auto writer
      { std::async
        ( std::launch::async
        , [&]
          {
            readers_started.wait();

            auto const lock {std::unique_lock (guard)};

            return readers_done == number_of_readers;
          }
        )
      };

    std::ranges::for_each
      ( readers
      , [] (auto& reader)
        {
          reader.get();
        }
      );

    ASSERT_TRUE (writer.get());
  }

  TYPED_TEST (MCSLockR, no_writer_starvation)
  {
    // Create a sequence of overlapping readers and one early writer:
    // The writer should not be delayed behind the last reader, which
    // means that not all readers see the initial value but some see
    // the updated value.
    //
    //     [-- read --]
    //        [-- read --]
    //           [-- read --]
    //              [-- read --]
    //                 [-- read --]
    //                    [-- read --]
    //       [-- write -- ]
    //

    auto guard {concurrency::SharedMutex<TypeParam>{}};

    auto const execution_time {std::chrono::milliseconds {30}};
    auto const create_delay {std::chrono::milliseconds {10}};
    auto const number_of_readers
      { std::invoke
        ( testing::random::value<int>
          { testing::random::value<int>::Min {20}
          , testing::random::value<int>::Max {100}
          }
        )
      };

    auto value {std::atomic<int> {0}};
    auto number_of_readers_created {std::atomic<int> {0}};

    auto const reader
      { [&]
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                auto const lock {std::shared_lock (guard)};

                std::this_thread::sleep_for (execution_time);

                return value.load();
              }
            );
        }
      };

    auto readers {std::vector<std::future<int>>{}};

    // asynchronous generation of a sequence of overlapping read
    auto reader_generator
      { std::async
        ( std::launch::async
        , [&]
          {
            for (auto r {0}; r != number_of_readers; ++r)
            {
              readers.emplace_back (reader());

              number_of_readers_created.fetch_add (1);
              number_of_readers_created.notify_one();

              std::this_thread::sleep_for (create_delay);
            }
          }
        )
      };

    // writer is generated after some but not all readers are created
    number_of_readers_created.wait (0);
    ASSERT_GT (number_of_readers_created, 0);
    ASSERT_LT (number_of_readers_created, number_of_readers);
    {
      auto const lock {std::unique_lock (guard)};

      // the writer takes some time itself and then modifies the value
      std::this_thread::sleep_for (execution_time);
      // use fetch_add (1) for atomics in general?
      value.fetch_add (1);
    }

    reader_generator.wait();

    auto const read_values
      { std::accumulate
        ( std::begin (readers), std::end (readers)
        , 0
        , [] (auto s, auto& r)
          {
            return s + r.get();
          }
        )
      };

    // Not all readers have seen the unmodified value even thought
    // they were created in an overlapping sequence -> the write has
    // priority over the not-yet created readers
    ASSERT_GT (read_values, 0);
  }

  TYPED_TEST (MCSLockR, no_reader_starvation)
  {
    // Create a sequence of overlapping writers and one early reader:
    // The reader should not be delayed behind the last writer, which
    // means that it sees some intermediate value, not the final value.
    //
    //     [-- write --]
    //        [-- write --]
    //           [-- write --]
    //              [-- write --]
    //                 [-- write --]
    //                    [-- write --]
    //       [-- read -- ]
    //

    auto guard {concurrency::SharedMutex<TypeParam>{}};

    auto const execution_time {std::chrono::milliseconds {30}};
    auto const create_delay {std::chrono::milliseconds {10}};
    auto const number_of_writers
      { std::invoke
        ( testing::random::value<int>
          { testing::random::value<int>::Min {20}
          , testing::random::value<int>::Max {100}
          }
        )
      };

    auto value {0};
    auto number_of_writers_created {std::atomic<int> {0}};

    auto const writer
      { [&]
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                auto const lock {std::unique_lock (guard)};

                std::this_thread::sleep_for (execution_time);

                ++value;
              }
            );
        }
      };

    auto writers {std::vector<std::future<void>>{}};

    // asynchronous generation of a sequence of overlapping write
    auto writer_generator
      { std::async
        ( std::launch::async
        , [&]
          {
            for (auto w {0}; w != number_of_writers; ++w)
            {
              writers.emplace_back (writer());

              number_of_writers_created.fetch_add (1);
              number_of_writers_created.notify_one();

              std::this_thread::sleep_for (create_delay);
            }
          }
        )
      };

    // reader is generated after some but not all writers are created
    number_of_writers_created.wait (0);
    ASSERT_GT (number_of_writers_created, 0);
    ASSERT_LT (number_of_writers_created, number_of_writers);
    auto const observed_value
      { std::invoke
        ( [&]
          {
            auto const lock {std::shared_lock (guard)};

            // the reader takes some time itself and then reads the value
            std::this_thread::sleep_for (execution_time);

            return value;
          }
        )
      };

    writer_generator.wait();

    std::ranges::for_each (writers, [] (auto& w) { w.get(); });

    ASSERT_EQ (value, number_of_writers);

    // The reader saw an intermediate value, not the final value.
    ASSERT_LT (observed_value, value);
  }

  namespace
  {
    class SharedResource
    {
    public:
      auto modify() -> void
      {
        _is_modified.store (true);
      }

      [[nodiscard]] auto is_modified() const -> bool
      {
        return _is_modified.load();
      }
    private:
      std::atomic<bool> _is_modified {false};
    };
  }

  TYPED_TEST (MCSLockR, scoped_lock_excludes_shared_lock)
  {
    auto mutex {concurrency::SharedMutex<TypeParam>{}};

    auto shared_resource {SharedResource{}};

    // Ensures that the reader does not try to acquire the lock
    // before the writer has acquired it.
    //
    auto lock_is_acquired {std::latch {1}};

    auto writer
      { std::async
        ( std::launch::async
        , [&]
          {
            auto const lock {std::scoped_lock (mutex)};

            lock_is_acquired.count_down();

            // If the lock does not provide mutual exclusion,
            // the reader will read the unmodified shared_resource
            // within this time window.
            // Which is the fail condition of this test.
            //
            std::this_thread::sleep_for (std::chrono::milliseconds {10});

            shared_resource.modify();

            return shared_resource.is_modified();
          }
        )
      };

    auto reader
      { std::async
        ( std::launch::async
        , [&]
          {
            lock_is_acquired.wait();

            auto const lock {std::shared_lock (mutex)};

            return shared_resource.is_modified();
          }
        )
      };

    ASSERT_TRUE (writer.get());
    ASSERT_TRUE (reader.get());
  }

  TYPED_TEST (MCSLockR, scoped_lock_excludes_unique_lock)
  {
    auto mutex {concurrency::SharedMutex<TypeParam>{}};

    auto shared_resource {SharedResource{}};

    auto lock_is_acquired {std::latch {1}};

    auto writer
      { std::async
        ( std::launch::async
        , [&]
          {
            auto const lock {std::scoped_lock (mutex)};

            lock_is_acquired.count_down();

            std::this_thread::sleep_for (std::chrono::milliseconds {10});

            shared_resource.modify();

            return shared_resource.is_modified();
          }
        )
      };

    auto reader
      { std::async
        ( std::launch::async
        , [&]
          {
            lock_is_acquired.wait();

            auto const lock {std::unique_lock (mutex)};

            return shared_resource.is_modified();
          }
        )
      };

    ASSERT_TRUE (writer.get());
    ASSERT_TRUE (reader.get());
  }

  TYPED_TEST (MCSLockR, deferred_unique_lock_does_not_own_on_construction)
  {
    auto guard {concurrency::SharedMutex<TypeParam>{}};

    auto const lock {std::unique_lock (guard, std::defer_lock)};

    ASSERT_FALSE (lock.owns_lock());
  }

  TYPED_TEST (MCSLockR, deferred_shared_lock_does_not_own_on_construction)
  {
    auto guard {concurrency::SharedMutex<TypeParam>{}};

    auto const lock {std::shared_lock (guard, std::defer_lock)};

    ASSERT_FALSE (lock.owns_lock());
  }

  TYPED_TEST (MCSLockR, deferred_shared_lock_allows_multiple_shared_locks)
  {
    auto mutex {concurrency::SharedMutex<TypeParam>{}};

    auto lock1 {std::shared_lock (mutex, std::defer_lock)};
    auto lock2 {std::shared_lock (mutex, std::defer_lock)};

    ASSERT_FALSE (lock1.owns_lock());
    ASSERT_FALSE (lock2.owns_lock());

    lock1.lock();
    ASSERT_TRUE (lock1.owns_lock());
    ASSERT_FALSE (lock2.owns_lock());

    lock2.lock();
    ASSERT_TRUE (lock1.owns_lock());
    ASSERT_TRUE (lock2.owns_lock());
  }

  TYPED_TEST (MCSLockR, deferred_unique_lock_excludes_shared_lock)
  {
    auto mutex {concurrency::SharedMutex<TypeParam>{}};

    auto shared_resource {SharedResource{}};

    auto lock_is_acquired {std::latch {1}};

    auto writer
      { std::async
        ( std::launch::async
        , [&]
          {
            auto lock {std::unique_lock (mutex, std::defer_lock)};
            EXPECT_FALSE (lock.owns_lock());

            lock.lock();
            EXPECT_TRUE (lock.owns_lock());

            lock_is_acquired.count_down();

            std::this_thread::sleep_for (std::chrono::milliseconds {10});

            shared_resource.modify();

            return shared_resource.is_modified();
          }
        )
      };

    auto reader
      { std::async
        ( std::launch::async
        , [&]
          {
            lock_is_acquired.wait();

            auto const lock {std::shared_lock (mutex)};

            return shared_resource.is_modified();
          }
        )
      };

    ASSERT_TRUE (writer.get());
    ASSERT_TRUE (reader.get());
  }

  // first_owner                                     second_owner
  // |                                               |
  // | lock (mutex)                                  | latch.wait
  // | latch.count_down()                            |
  // |   releases latch for second_owner  ==>        | lock (mutex)
  // |                                               |   waits for first_owner to release mutex
  // |                                               |   :
  // | sleep_for (10ms)                              |   :
  // |   ensures second_owner is waiting for mutex   |   :
  // |                                               |   :
  // | first_owner_resource.modify()                 |   :
  // |   gives second_owner something to observe     |   :
  // |                                               |   :
  // | unlock (mutex)                                |   :
  // |   allows second_owner to acquire mutex        |   acquires mutex
  // |                                               |
  // | lock (mutex)                                  | tests modification of first_owner_resource
  // |    waits for second_owner to release mutex    |
  // |    :                                          | sleep_for (10ms)
  // |    :                                          |   ensures first_owner is waiting for mutex
  // |    :                                          |
  // |    :                                          | second_owner_resource.modify()
  // |    :                                          |   gives first_owner something to observe
  // |    :                                          |
  // |    :                                          | return
  // |    : acquire mutex                            |   unlocks mutex
  // |                                               |
  // | return                                        |
  //
  TYPED_TEST
    ( MCSLockR
    , Lock_on_SharedMutex_can_be_acquired_and_released_multiple_times
    )
  {
    auto mutex {concurrency::SharedMutex<TypeParam>{}};

    auto first_owner_resource {SharedResource{}};
    auto second_owner_resource {SharedResource{}};

    auto lock_is_acquired {std::latch {1}};

    auto first_owner
      { std::async
        ( std::launch::async
        , [&]
          {
            auto lock {std::unique_lock (mutex)};

            lock_is_acquired.count_down();

            std::this_thread::sleep_for (std::chrono::milliseconds {10});

            first_owner_resource.modify();

            lock.unlock();

            // Ensures the second owner has acquired the lock.
            // This is not necessary for FIFO.
            //
            std::this_thread::sleep_for (std::chrono::milliseconds {10});

            // now the second owner can acquire the lock and observe
            // the modification of first_owner_resource.
            // After that the second owner modifies second_owner_resource,
            // which is observed by the first owner after it reacquires
            // the lock.
            //
            lock.lock();

            return (  first_owner_resource.is_modified()
                   && second_owner_resource.is_modified()
                   );
          }
        )
      };

    auto second_owner
      { std::async
        ( std::launch::async
        , [&]
          {
            lock_is_acquired.wait();

            auto const lock {std::shared_lock (mutex)};

            if (!first_owner_resource.is_modified())
            {
              return false;
            }

            std::this_thread::sleep_for (std::chrono::milliseconds {10});

            second_owner_resource.modify();

            return (  first_owner_resource.is_modified()
                   && second_owner_resource.is_modified()
                   );
          }
        )
      };

    ASSERT_TRUE (first_owner.get());
    ASSERT_TRUE (second_owner.get());
  }

  TYPED_TEST (MCSLockR, deferred_shared_lock_excludes_unique_lock)
  {
    auto mutex {concurrency::SharedMutex<TypeParam>{}};

    auto shared_resource {SharedResource{}};

    auto lock_is_acquired {std::latch {1}};

    auto writer
      { std::async
        ( std::launch::async
        , [&]
          {
            auto lock {std::shared_lock (mutex, std::defer_lock)};
            EXPECT_FALSE (lock.owns_lock());

            lock.lock();
            EXPECT_TRUE (lock.owns_lock());

            lock_is_acquired.count_down();

            std::this_thread::sleep_for (std::chrono::milliseconds {10});

            shared_resource.modify();

            return shared_resource.is_modified();
          }
        )
      };

    auto reader
      { std::async
        ( std::launch::async
        , [&]
          {
            lock_is_acquired.wait();

            auto const lock {std::unique_lock (mutex)};

            return shared_resource.is_modified();
          }
        )
      };

    ASSERT_TRUE (writer.get());
    ASSERT_TRUE (reader.get());
  }
}

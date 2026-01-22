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
#include <mcs/util/Lock.hpp>
#include <mcs/util/lock/queue/FIFO.hpp>
#include <mcs/util/lock/queue/Fast.hpp>
#include <memory>
#include <thread>

namespace mcs::util
{
  namespace
  {
    using Queues = ::testing::Types
      < lock::queue::FIFO
      , lock::queue::Fast
      >;

    template<class> struct MCSLockR : public testing::random::Test{};
    TYPED_TEST_SUITE (MCSLockR, Queues);
  }

  namespace lock_write_access_example_from_documentation_compiles
  {
    // EXAMPLE:
    //
    // Wrap an UnsynchronizedContainer into a SynchronizedContainer with
    // shared read access and unique write access.

    using LockQueueType = lock::queue::FIFO;

    struct UnsynchronizedContainer
    {
      auto cm() const -> void;
      auto mm() -> void;
    };

    struct SynchronizedContainer : private UnsynchronizedContainer
    {
      using UnsynchronizedContainer::UnsynchronizedContainer;

      struct ReadAccess
      {
        auto cm() const
        {
          return _unsynchronized_container->cm();
        }

      private:
        friend struct SynchronizedContainer;
        Lock<lock::mode::Shared, LockQueueType> _lock;
        UnsynchronizedContainer const* _unsynchronized_container;
        template<typename... LockArgs>
          ReadAccess
            ( UnsynchronizedContainer const* unsynchronized_container
            , LockArgs&&... lock_args
            )
            : _lock {std::forward<LockArgs> (lock_args)...}
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
        Lock<lock::mode::Unique, LockQueueType> _lock;
        UnsynchronizedContainer* _unsynchronized_container;
        template<typename... LockArgs>
          WriteAccess
            ( UnsynchronizedContainer* unsynchronized_container
            , LockArgs&&... lock_args
            )
            : _lock {std::forward<LockArgs> (lock_args)...}
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
      lock::SharedMutex<LockQueueType> _guard;
    };
  }

  namespace lock_read_write_access_example_from_documentation_compiles
  {
    // EXAMPLE:
    //
    // Wrap an UnsynchronizedContainer into a SynchronizedContainer with
    // shared read access and unique read&write access.
    //

    using LockQueueType = lock::queue::Fast;

    struct UnsynchronizedContainer
    {
      auto cm() const -> void;
      auto mm() -> void;
    };

    struct SynchronizedContainer : private UnsynchronizedContainer
    {
      using UnsynchronizedContainer::UnsynchronizedContainer;

      template<lock::is_mode Mode>
        struct Access
      {
        auto cm() const
        {
          return _unsynchronized_container->cm();
        }

      private:
        friend struct SynchronizedContainer;

        Lock<Mode, LockQueueType> _lock;
        UnsynchronizedContainer const* _unsynchronized_container;

        template<typename... LockArgs>
          Access
            ( UnsynchronizedContainer const* unsynchronized_container
            , LockArgs&&... lock_args
            )
            : _lock {std::forward<LockArgs> (lock_args)...}
            , _unsynchronized_container {unsynchronized_container}
        {}
      };
      using ReadAccess = Access<lock::mode::Shared>;

      struct ReadWriteAccess : public Access<lock::mode::Unique>
      {
        using Access<lock::mode::Unique>::cm;
        auto mm() const
        {
          return _unsynchronized_container->mm();
        }

      private:
        friend struct SynchronizedContainer;
        UnsynchronizedContainer* _unsynchronized_container;
        template<typename... LockArgs>
          ReadWriteAccess
            ( UnsynchronizedContainer* unsynchronized_container
            , LockArgs&&... lock_args
            )
              : Access<lock::mode::Unique>
                { unsynchronized_container
                , std::forward<LockArgs> (lock_args)...
                }
              , _unsynchronized_container {unsynchronized_container}
        {}
      };

      [[nodiscard]] auto read_access() const -> ReadAccess
      {
        return ReadAccess {this, std::addressof (_guard)};
      }
      [[nodiscard]] auto write_access() -> ReadWriteAccess
      {
        return ReadWriteAccess {this, std::addressof (_guard)};
      }
    private:
      lock::SharedMutex<LockQueueType> _guard;
    };
  }

  TYPED_TEST (MCSLockR, a_guard_can_have_multiple_shared_locks_at_the_same_time)
  {
    auto const guard {lock::SharedMutex<TypeParam>{}};

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
                  auto const lock {shared_lock (guard)};

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
    auto guard {lock::SharedMutex<TypeParam>{}};

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
                  auto const lock {unique_lock (guard)};

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
    auto guard {lock::SharedMutex<TypeParam>{}};

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
                auto const lock {shared_lock (guard)};

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

            auto const lock {unique_lock (guard)};

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

    auto guard {lock::SharedMutex<TypeParam>{}};

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
                auto const lock {shared_lock (guard)};

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

              std::this_thread::sleep_for (create_delay);
            }
          }
        )
      };

    std::this_thread::sleep_for (create_delay);

    // writer is generated after some but not all readers are created
    ASSERT_GT (number_of_readers_created, 0);
    ASSERT_LT (number_of_readers_created, number_of_readers);
    {
      auto const lock {unique_lock (guard)};

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

    auto guard {lock::SharedMutex<TypeParam>{}};

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
                auto const lock {unique_lock (guard)};

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

              std::this_thread::sleep_for (create_delay);
            }
          }
        )
      };

    std::this_thread::sleep_for (create_delay);

    // reader is generated after some but not all writers are created
    ASSERT_GT (number_of_writers_created, 0);
    ASSERT_LT (number_of_writers_created, number_of_writers);
    auto const observed_value
      { std::invoke
        ( [&]
          {
            auto const lock {shared_lock (guard)};

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
}

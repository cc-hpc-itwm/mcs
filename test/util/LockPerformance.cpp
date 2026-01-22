// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <barrier>
#include <fmt/base.h>
#include <fmt/ranges.h>
#include <future>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/util/Lock.hpp>
#include <mcs/util/lock/queue/FIFO.hpp>
#include <mcs/util/lock/queue/Fast.hpp>
#include <mcs/util/timed.hpp>
#include <memory>
#include <mutex>
#include <numeric>
#include <shared_mutex>
#include <thread>
#include <vector>

namespace mcs::util
{
  namespace
  {
    struct STDMutex
    {
      static auto constexpr description {"std::mutex"};

      using Guard = std::mutex;

      static auto unique_lock (Guard& guard)
      {
        return std::lock_guard {guard};
      }
      static auto shared_lock (Guard& guard)
      {
        return std::lock_guard {guard};
      }
    };

    struct STDSharedMutex
    {
      static auto constexpr description {"std::shared_mutex"};

      using Guard = std::shared_mutex;

      static auto unique_lock (Guard& guard)
      {
        return std::unique_lock {guard};
      }
      static auto shared_lock (Guard& guard)
      {
        return std::shared_lock {guard};
      }
    };

    struct LockGuardFIFO
    {
      static auto constexpr description
        { "util::lock::SharedMutex<queue::FIFO>"
        };

      using Guard = lock::SharedMutex<lock::queue::FIFO>;

      static auto unique_lock (Guard& guard)
      {
        return util::unique_lock (guard);
      }
      static auto shared_lock (Guard& guard)
      {
        return util::shared_lock (guard);
      }
    };

    struct LockGuardFast
    {
      static auto constexpr description
        { "util::lock::SharedMutex<queue::Fast>"
        };

      using Guard = lock::SharedMutex<lock::queue::Fast>;

      static auto unique_lock (Guard& guard)
      {
        return util::unique_lock (guard);
      }
      static auto shared_lock (Guard& guard)
      {
        return util::shared_lock (guard);
      }
    };

    using Implementations = ::testing::Types
      < STDMutex
      , STDSharedMutex
      , LockGuardFIFO
      , LockGuardFast
      >;
    template<class> struct MCSLockPerformanceT : ::testing::Test{};
    TYPED_TEST_SUITE (MCSLockPerformanceT, Implementations);
  }

  TYPED_TEST (MCSLockPerformanceT, concurrent_increment)
  {
    static constexpr auto clock_scale {100'000L};

    auto guard {typename TypeParam::Guard{}};

    auto n {0UL};
    auto const M {10'000};
    auto const T {std::thread::hardware_concurrency()};

    auto workers {std::vector<std::future<timer::Ticks<clock_scale>>>{}};
    workers.reserve (T);

    auto barrier {std::barrier {T}};

    std::generate_n
      ( std::back_inserter (workers)
      , T
      , [&]() noexcept
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                barrier.arrive_and_wait();

                return timed<clock_scale>
                  ( [&]() noexcept
                    {
                      for (auto i {0}; i != M; ++i)
                      {
                        auto const lock {TypeParam::unique_lock (guard)};

                        ++n;
                      }
                    }
                  );
              }
            );
        }
      );

    auto durations {std::vector<long>{}};
    durations.reserve (T);

    std::ranges::for_each
      ( workers
      , [&] (auto& worker)
        {
          durations.emplace_back (worker.get().value());
        }
      );

    ASSERT_EQ (n, T * M);

    auto const [min, max] {std::ranges::minmax_element (durations)};

    fmt::print
      ( "concurrent_increment: {}: M {} T {}: total {} min {} max {} * sec/{}\n"
      , TypeParam::description
      , M
      , T
      , std::reduce (std::begin (durations), std::end (durations))
      , *min
      , *max
      , clock_scale
      );
  }

  TYPED_TEST (MCSLockPerformanceT, concurrent_read)
  {
    static constexpr auto clock_scale {100'000L};

    auto guard {typename TypeParam::Guard{}};

    auto const M {10'000};
    auto const T {std::thread::hardware_concurrency()};

    auto workers {std::vector<std::future<timer::Ticks<clock_scale>>>{}};
    workers.reserve (T);

    auto barrier {std::barrier {T}};

    std::generate_n
      ( std::back_inserter (workers)
      , T
      , [&]() noexcept
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                barrier.arrive_and_wait();

                return timed<clock_scale>
                  ( [&]() noexcept
                    {
                      for (auto i {0}; i != M; ++i)
                      {
                        auto const lock {TypeParam::shared_lock (guard)};
                      }
                    }
                  );
              }
            );
        }
      );

    auto durations {std::vector<long>{}};
    durations.reserve (T);

    std::ranges::for_each
      ( workers
      , [&] (auto& worker)
        {
          durations.emplace_back (worker.get().value());
        }
      );

    auto const [min, max] {std::ranges::minmax_element (durations)};

    fmt::print
      ( "concurrent_read: {}: M {} T {}: total {} min {} max {} * sec/{}\n"
      , TypeParam::description
      , M
      , T
      , std::reduce (std::begin (durations), std::end (durations))
      , *min
      , *max
      , clock_scale
      );
  }

  TYPED_TEST (MCSLockPerformanceT, concurrent_read_and_concurrent_write)
  {
    static constexpr auto clock_scale {100'000L};

    auto guard {typename TypeParam::Guard{}};

    auto n {0UL};
    auto const M {10'000};
    auto const T {std::thread::hardware_concurrency()};

    auto workers {std::vector<std::future<timer::Ticks<clock_scale>>>{}};
    workers.reserve (T);

    auto barrier {std::barrier {T + T}};

    std::generate_n
      ( std::back_inserter (workers)
      , T
      , [&]() noexcept
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                barrier.arrive_and_wait();

                return timed<clock_scale>
                  ( [&]() noexcept
                    {
                      for (auto i {0}; i != M; ++i)
                      {
                        auto const lock {TypeParam::unique_lock (guard)};

                        ++n;
                      }
                    }
                  );
              }
            );
        }
      );

    std::generate_n
      ( std::back_inserter (workers)
      , T
      , [&]() noexcept
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                barrier.arrive_and_wait();

                return timed<clock_scale>
                  ( [&]() noexcept
                    {
                      for (auto i {0}; i != M; ++i)
                      {
                        auto const lock {TypeParam::shared_lock (guard)};
                      }
                    }
                  );
              }
            );
        }
      );

    auto durations {std::vector<long>{}};
    durations.reserve (T);

    std::ranges::for_each
      ( workers
      , [&] (auto& worker)
        {
          durations.emplace_back (worker.get().value());
        }
      );

    ASSERT_EQ (n, T * M);

    auto const [min, max] {std::ranges::minmax_element (durations)};

    fmt::print
      ( "concurrent_read&write: {}: M {} T {}: total {} min {} max {} * sec/{}\n"
      , TypeParam::description
      , M
      , T
      , std::reduce (std::begin (durations), std::end (durations))
      , *min
      , *max
      , clock_scale
      );
  }
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <barrier>
#include <chrono>
#include <cstdint>
#include <future>
#include <gtest/gtest.h>
#include <iterator>
#include <list>
#include <mcs/iov_backend/detail/Bitmap.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/random_device.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/cast.hpp>
#include <numeric>
#include <ranges>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
  template<std::integral I>
    auto milliseconds (I n)
  {
    return std::chrono::system_clock::now() + std::chrono::milliseconds {n};
  }
}

namespace mcs::iov_backend::detail
{
  namespace
  {
    struct IOV_BackendDetailBitmapR : public testing::random::Test
    {
      using RandomIndex = testing::random::value<std::size_t>;

      // \note 16M many units of size 16K are 256G of communication buffer
      RandomIndex random_size
        { RandomIndex::Min {0}
        , RandomIndex::Max {16 << 20}
        };
    };
  }

  TEST_F (IOV_BackendDetailBitmapR, size_returns_constructor_parameter)
  {
    auto const size {random_size()};

    ASSERT_EQ (Bitmap {size}.size(), size);
  }

  TEST_F (IOV_BackendDetailBitmapR, size_zero_is_possible_and_set_will_time_out)
  {
    auto bitmap {Bitmap {0}};

    testing::require_exception
      ( [&]
        {
          std::ignore =
            bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
            ;
        }
      , testing::assert_type<Bitmap::Error::Set::Timeout>()
      );
  }

  namespace
  {
    struct IOV_BackendDetailRandomBitmapR : public IOV_BackendDetailBitmapR
    {
      Bitmap bitmap {random_size()};
    };
  }

  TEST_F (IOV_BackendDetailRandomBitmapR, size_many_bits_can_be_set)
  {
    auto set_bits {std::unordered_set<Bitmap::Index>{}};

    std::ranges::for_each
      ( std::views::iota (Bitmap::Index {0}, bitmap.size())
      , [&] (auto index)
        {
          std::ignore = index;

          auto const bit
            { bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
            };

          ASSERT_TRUE (set_bits.emplace (bit).second);
        }
      );
  }

  TEST_F ( IOV_BackendDetailRandomBitmapR
         , more_than_size_many_bits_can_not_be_set
         )
  {
    std::ranges::for_each
      ( std::views::iota (Bitmap::Index {0}, bitmap.size())
      , [&] (auto index)
        {
          std::ignore = index;

          std::ignore =
            bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
            ;
        }
      );

    testing::require_exception
      ( [&]
        {
          std::ignore =
            bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
            ;
        }
      , testing::assert_type<Bitmap::Error::Set::Timeout>()
      );
  }

  TEST_F ( IOV_BackendDetailRandomBitmapR
         , size_many_bits_can_be_set_and_cleared_in_any_order
         )
  {
    auto set_bits {std::vector<Bitmap::Index>{}};

    std::ranges::for_each
      ( std::views::iota (Bitmap::Index {0}, bitmap.size())
      , [&] (auto index)
        {
          std::ignore = index;

          auto const bit
            { bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
            };

          set_bits.push_back (bit);
        }
      );

    std::ranges::shuffle (set_bits, testing::random::random_device());

    for (auto bit : set_bits)
    {
      bitmap.clear (bit);
    }

    std::ranges::for_each
      ( std::views::iota (Bitmap::Index {0}, bitmap.size())
      , [&] (auto index)
        {
          std::ignore = index;

          std::ignore =
            bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
            ;
        }
      );
  }

  TEST_F (IOV_BackendDetailRandomBitmapR, set_can_be_interrupted)
  {
    // Set all bits to make sure that subsequent calls to set will not
    // find a bit and block until timeout or interruption happens.
    std::ranges::for_each
      ( std::views::iota (Bitmap::Index {0}, bitmap.size())
      , [&] (auto index)
        {
          std::ignore = index;

          std::ignore =
            bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
            ;
        }
      );

    // No more bits are available.
    testing::require_exception
      ( [&]
        {
          std::ignore =
            bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
            ;
        }
      , testing::assert_type<Bitmap::Error::Set::Timeout>()
      );

    auto interruption_context {Bitmap::InterruptionContext{}};

    auto async_set
      { std::async
        ( std::launch::async
        , [&]
          {
            // Try to acquire a bit patiently...
            return bitmap.set (interruption_context, milliseconds (10'000));
          }
        )
      };

    // ...but now loose patience and interrupt the ongoing call:
    // \note The interruption might happen before or after the call to
    // set has started...
    //
    bitmap.interrupt (interruption_context);

    testing::require_exception
      ( [&]
        {
          std::ignore = async_set.get();
        }
      , testing::assert_type<Bitmap::Error::Set::Interrupted>()
      );
  }

  TEST_F ( IOV_BackendDetailRandomBitmapR
         , multiple_threads_can_concurrently_work
         )
  {
    auto const number_of_threads
      { testing::random::value<std::size_t> {2, 10}()
      };

    auto const number_of_set_clear
      { std::invoke
        ( [&]
          {
            auto _number_of_set_clear
              {std::vector<std::size_t> (number_of_threads)};

            std::ranges::generate
              ( _number_of_set_clear
              , testing::random::value<std::size_t> {1 << 10, 1 << 20}
              );

            return _number_of_set_clear;
          }
        )
      };

    auto threads {std::list<std::future<std::size_t>>{}};

    // All threads wait until all are started in order to maximise
    // parallelism.
    //
    auto barrier
      { std::barrier {util::cast<std::ptrdiff_t> (number_of_threads)}
      };

    std::ranges::transform
      ( number_of_set_clear
      , std::back_inserter (threads)
      , [&] (auto n)
        {
          return std::async
            ( std::launch::async
            , [this, &barrier, n]
              {
                barrier.arrive_and_wait();

                std::ranges::for_each
                  ( std::views::iota (std::size_t {0}, n)
                  , [&] (auto index)
                    {
                      std::ignore = index;

                  bitmap.clear
                    ( bitmap.set
                        ( Bitmap::InterruptionContext{}
                        , milliseconds (10'000)
                        )
                    );
                }
                  );

                return n;
              }
            );
        }
      );

    ASSERT_EQ
      ( std::accumulate
        ( std::begin (number_of_set_clear), std::end (number_of_set_clear)
        , std::size_t {0}
        )
      , std::accumulate
        ( std::begin (threads), std::end (threads)
        , std::size_t {0}
        , [] (auto sum, auto& thread)
          {
            return sum + thread.get();
          }
        )
      );
  }

  namespace
  {
    struct IOV_BackendDetailRandomBitmapTimeR
      : public IOV_BackendDetailRandomBitmapR
    {
      using Clock = std::chrono::steady_clock;

      decltype (Clock::now()) _start;

      auto start()
      {
        _start = Clock::now();
      }
      template<typename Description>
        auto stop (Description description) const
      {
        auto const end {Clock::now()};
        auto const µs
          { std::chrono::duration_cast<std::chrono::microseconds> (end - _start)
          };
        auto const ns
          { std::chrono::duration_cast<std::chrono::nanoseconds> (end - _start)
          };

        fmt::print ( "time {}: {} in {} µs -> {} M/sec, {} ns/entry\n"
                   , description
                   , bitmap.size()
                   , µs.count()
                   , bitmap.size() / util::cast<std::size_t> (µs.count())
                   , util::cast<std::size_t> (ns.count()) / bitmap.size()
                   );
      }
    };
  }

  TEST_F (IOV_BackendDetailRandomBitmapTimeR, time_to_set_size_many_bits)
  {
    start();

    for (auto i {Bitmap::Index {0}}; i != bitmap.size(); ++i)
    {
      std::ignore
        = bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
        ;
    }

    stop ("set");
  }

  TEST_F ( IOV_BackendDetailRandomBitmapTimeR
         , time_to_clear_size_many_bits_in_random_order
         )
  {
    auto set_bits {std::vector<Bitmap::Index>{}};

    for (auto i {Bitmap::Index {0}}; i != bitmap.size(); ++i)
    {
      auto const bit
        { bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
        };

      set_bits.push_back (bit);
    }

    std::ranges::shuffle (set_bits, testing::random::random_device());

    start();

    for (auto bit : set_bits)
    {
      bitmap.clear (bit);
    }

    stop ("clear");
  }

  TEST_F ( IOV_BackendDetailRandomBitmapTimeR
         , time_to_set_and_clear_size_many_bits
         )
  {
    start();

    for (auto i {Bitmap::Index {0}}; i != bitmap.size(); ++i)
    {
      bitmap.clear
        ( bitmap.set (Bitmap::InterruptionContext{}, milliseconds (0))
        );
    }

    stop ("set_and_clear");
  }

  // \note test/util/Queue demonstrates usage of error() and
  // interrupt() in detail
}

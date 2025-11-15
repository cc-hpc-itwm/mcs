// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <array>
#include <barrier>
#include <chrono>
#include <compare>
#include <cstddef>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <iterator>
#include <list>
#include <mcs/Error.hpp>
#include <mcs/iov_backend/detail/Queue.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/random_device.hpp>
#include <mcs/testing/random/unique_values.hpp>
#include <mcs/testing/random/value/bool.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/overloaded.hpp>
#include <numeric>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace
{
  template<typename I>
    auto milliseconds_from_now (I i)
  {
    return std::chrono::steady_clock::now() + std::chrono::milliseconds {i};
  }
}

namespace mcs::iov_backend::detail
{
  struct IOVBackendDetailQueue : public testing::random::Test{};

  TEST_F ( IOVBackendDetailQueue
         , nothing_to_get_from_an_empty_queue
         )
  {
    auto queue {Queue<int>{}};

    auto const interruption_context {decltype (queue)::InterruptionContext{}};

    testing::require_exception
      ( [&]
        {
          std::ignore = queue.get
            ( interruption_context
            , milliseconds_from_now (1)
            );
        }
      , testing::assert_type<typename decltype (queue)::Error::Get::Timeout>()
      );
  }

  TEST_F ( IOVBackendDetailQueue
         , queue_is_fifo
         )
  {
    auto const xs
      { std::invoke
        ( []
          {
            auto _xs {std::vector<int>{}};
            using Size = typename decltype (_xs)::size_type;
            std::generate_n
              ( std::back_inserter (_xs)
              , testing::random::value<Size> {Size {0}, Size {1000}}()
              , testing::random::value<int>{}
              );

            return _xs;
          }
        )
      };

    auto queue {Queue<int>{}};

    auto const interruption_context {decltype (queue)::InterruptionContext{}};

    auto const get
      { [&]
        {
          return queue.get
            ( interruption_context
            , milliseconds_from_now (1)
            );
        }
      };

    std::ranges::for_each
      ( xs
      , [&] (auto x)
        {
          queue.push (x);
        }
      );

    std::ranges::for_each
      ( xs
      , [&] (auto x)
        {
          auto const q {get()};
          ASSERT_EQ (q, x);
        }
      );

    testing::require_exception
      ( [&]
        {
          std::ignore = get();
        }
      , testing::assert_type<typename decltype (queue)::Error::Get::Timeout>()
      );
  }

  TEST_F ( IOVBackendDetailQueue
         , queue_is_fifo_and_can_be_used_by_producer_and_consumer_concurrently
         )
  {
    auto const xs
      { std::invoke
        ( []
          {
            auto _xs {std::vector<int>{}};
            using Size = typename decltype (_xs)::size_type;
            std::generate_n
              ( std::back_inserter (_xs)
              , testing::random::value<Size> {Size {0}, Size {1000}}()
              , testing::random::value<int>{}
              );

            return _xs;
          }
        )
      };

    auto queue {Queue<int>{}};

    // producer and consumer are all waiting until both of them are
    // running and only then begin to work, this is meant to maximize
    // parallel work
    //
    auto barrier {std::barrier {2}};

    auto const producer
      { std::async
        ( std::launch::async
        , [&xs, &barrier, &queue]
          {
            barrier.arrive_and_wait();

            std::ranges::for_each
              ( xs
              , [&] (auto x)
                {
                  queue.push (x);
                }
              );
          }
        )
      };

    auto const consumer
      { std::async
        ( std::launch::async
        , [&xs, &barrier, &queue]
          {
            barrier.arrive_and_wait();

            std::ranges::for_each
              ( xs
              , [&queue] (auto x)
                {
                  auto const interruption_context
                    {decltype (queue)::InterruptionContext{}};

                  auto const get_result
                    { queue.get
                        ( interruption_context
                        , milliseconds_from_now (15000)
                        )
                    };

                  ASSERT_EQ (get_result, x);
                }
              );
          }
        )
      };

    producer.wait();
    consumer.wait();

    {
      auto const interruption_context {decltype (queue)::InterruptionContext{}};

      testing::require_exception
        ( [&]
          {
            std::ignore = queue.get
              ( interruption_context
              , milliseconds_from_now (1)
              );
          }
        , testing::assert_type<typename decltype (queue)::Error::Get::Timeout>()
        );
    }
  }

  TEST_F ( IOVBackendDetailQueue
         , get_call_can_be_interrupted
         )
  {
    auto queue {Queue<int>{}};

    auto interruption_context {decltype (queue)::InterruptionContext{}};

    auto async_get
      { std::async
        ( std::launch::async
        , [&]
          {
            return queue.get
              ( interruption_context
              , milliseconds_from_now (1000)
              );
          }
        )
      };

    queue.interrupt (interruption_context);

    testing::require_exception
      ( [&]
        {
          std::ignore = async_get.get();
        }
      , testing::assert_type<typename decltype (queue)::Error::Get::Interrupted>()
      );
  }

  TEST_F ( IOVBackendDetailQueue
         , interrupted_interruption_context_interrupts_immediately
         )
  {
    auto queue {Queue<int>{}};

    auto interruption_context {decltype (queue)::InterruptionContext{}};

    queue.interrupt (interruption_context);

    testing::require_exception
      ( [&]
        {
          std::ignore = queue.get
            ( interruption_context
            , milliseconds_from_now (1)
            );
        }
      , testing::assert_type<typename decltype (queue)::Error::Get::Interrupted>()
      );
  }

  TEST_F ( IOVBackendDetailQueue
         , multiple_get_can_share_one_interruption_context
         )
  {
    auto queue {Queue<int>{}};

    auto interruption_context {decltype (queue)::InterruptionContext{}};

    auto gets {std::list<std::future<int>>{}};

    using Size = typename decltype (gets)::size_type;

    std::generate_n
      ( std::back_inserter (gets)
      , testing::random::value<Size> {Size {0}, Size {20}}()
      , [&]
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                return queue.get
                  ( interruption_context
                  , milliseconds_from_now (1000)
                  );
              }
            );
        }
      );

    queue.interrupt (interruption_context);

    for (auto& async_get : gets)
    {
      testing::require_exception
        ( [&]
          {
            std::ignore = async_get.get();
          }
        , testing::assert_type<typename decltype (queue)::Error::Get::Interrupted>()
        );
    }
  }

  TEST_F ( IOVBackendDetailQueue
         , interruption_does_not_affect_gets_that_use_a_different_context
         )
  {
    auto queue {Queue<int>{}};

    using Gets = std::list<std::future<int>>;

    using Size = typename Gets::size_type;

    auto random_size {testing::random::value<Size> {Size {0}, Size {20}}};

    struct GetsWithInterruptionContext
    {
      decltype (queue)::InterruptionContext interruption_context{};
      Gets gets{};
    };

    // Create two sets of gets, each set using a separate
    // interruption_context
    //
    auto gets_with_interruption_context
      {std::array<GetsWithInterruptionContext, 2>{}};

    using Slot = typename decltype (gets_with_interruption_context)::size_type;

    for (auto& gets : gets_with_interruption_context)
    {
      std::generate_n
        ( std::back_inserter (gets.gets)
        , random_size()
        , [&]
          {
            return std::async
              ( std::launch::async
              , [&]
                {
                  return queue.get
                    ( gets.interruption_context
                    , milliseconds_from_now (1000)
                    );
                }
              );
          }
        );
    }

    // Interrupt one of the sets and expect all their calls to be
    // interrupted.
    //
    auto const slot
      { std::invoke
        ( []
          {
            auto _slot {std::vector<Slot> {Slot {0}, Slot {1}}};
            std::ranges::shuffle
              ( _slot
              , testing::random::random_device()
              );
            return _slot;
          }
        )
      };


    {
      auto& gets {gets_with_interruption_context.at (slot.at (0))};

      queue.interrupt (gets.interruption_context);

      for (auto& async_get : gets.gets)
      {
        testing::require_exception
          ( [&]
            {
              std::ignore = async_get.get();
            }
          , testing::assert_type<typename decltype (queue)::Error::Get::Interrupted>()
          );
      }
    }

    // Put enough elements into the queue for the non-interrupted set
    // to get them and expect the whole set of gets to be alive.
    //
    {
      auto& gets {gets_with_interruption_context.at (slot.at (1))};

      auto put_values {std::vector<int>{}};

      std::generate_n
        ( std::back_inserter (put_values)
        , gets.gets.size()
        , testing::random::value<int>{}
        );

      std::ranges::for_each
        ( put_values
        , [&] (auto value)
          {
            queue.push (value);
          }
        );

      auto get_values {std::vector<int>{}};

      for (auto& async_get : gets.gets)
      {
        auto const r {async_get.get()};

        get_values.emplace_back (r);
      }

      // sort the put and get values in order to be independent from
      // thread schedule
      //
      std::ranges::sort (put_values);
      std::ranges::sort (get_values);

      ASSERT_EQ (put_values, get_values);
    }
  }

  namespace
  {
    struct Error : public mcs::Error
    {
      Error (int i)
        : mcs::Error {fmt::format ("Error {}", i)}
        , _i {i}
      {}
      ~Error() override = default;
      Error (Error const&) = default;
      Error (Error&&) = default;
      auto operator= (Error const&) -> Error& = default;
      auto operator= (Error&&) -> Error& = default;

      int _i;

      [[nodiscard]] constexpr auto operator==
        ( Error const& other
        ) const noexcept
      {
        return _i == other._i;
      }
    };
  }

  TEST_F ( IOVBackendDetailQueue
         , error_is_sticky_and_future_calls_to_error_will_throw_the_first_error
         )
  {
    auto random_int {testing::random::value<int>{}};
    auto const error {Error {random_int()}};

    auto queue {Queue<int>{}};

    queue.error (std::make_exception_ptr (error));

    testing::require_exception
      ( [&]
        {
          queue.error (std::make_exception_ptr (Error {random_int()}));
        }
      , testing::Assert<Error>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught, error);
          }
        }
      );
  }

  TEST_F ( IOVBackendDetailQueue
         , error_is_sticky_and_future_calls_to_push_will_throw_the_error
         )
  {
    auto random_int {testing::random::value<int>{}};
    auto const error {Error {random_int()}};

    auto queue {Queue<int>{}};

    queue.error (std::make_exception_ptr (error));

    testing::require_exception
      ( [&]
        {
          queue.push (random_int());
        }
      , testing::Assert<Error>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught, error);
          }
        }
      );
  }

  TEST_F ( IOVBackendDetailQueue
         , error_is_sticky_and_future_calls_to_interrupt_will_throw_the_error
         )
  {
    auto random_int {testing::random::value<int>{}};
    auto const error {Error {random_int()}};

    auto queue {Queue<int>{}};

    queue.error (std::make_exception_ptr (error));

    testing::require_exception
      ( [&]
        {
          auto interruption_context {decltype (queue)::InterruptionContext{}};
          queue.interrupt (interruption_context);
        }
      , testing::Assert<Error>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught, error);
          }
        }
      );
  }

  TEST_F ( IOVBackendDetailQueue
         , error_is_sticky_and_future_calls_to_get_of_empty_queue_will_throw_the_error
         )
  {
    auto random_int {testing::random::value<int>{}};
    auto const error {Error {random_int()}};

    auto queue {Queue<int>{}};

    auto interruption_context {decltype (queue)::InterruptionContext{}};

    if (testing::random::value<bool>{}())
    {
      queue.interrupt (interruption_context);
    }

    queue.error (std::make_exception_ptr (error));

    testing::require_exception
      ( [&]
        {
          std::ignore = queue.get
            ( interruption_context
            , milliseconds_from_now (random_int())
            );
        }
      , testing::Assert<Error>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught, error);
          }
        }
      );
  }

  TEST_F ( IOVBackendDetailQueue
         , error_is_sticky_and_future_calls_to_get_of_nonempty_queue_will_throw_the_error
         )
  {
    auto random_int {testing::random::value<int>{}};
    auto const error {Error {random_int()}};

    auto queue {Queue<int>{}};

    auto interruption_context {decltype (queue)::InterruptionContext{}};

    if (testing::random::value<bool>{}())
    {
      queue.interrupt (interruption_context);
    }

    queue.push (random_int());

    queue.error (std::make_exception_ptr (error));

    testing::require_exception
      ( [&]
        {
          std::ignore = queue.get
            ( interruption_context
            , milliseconds_from_now (random_int())
            );
        }
      , testing::Assert<Error>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught, error);
          }
        }
      );
  }

  TEST_F ( IOVBackendDetailQueue
         , error_is_sticky_and_ongoing_calls_to_get_will_throw_the_error
         )
  {
    auto random_int {testing::random::value<int>{}};
    auto const error {Error {random_int()}};

    auto queue {Queue<int>{}};

    auto interruption_context {decltype (queue)::InterruptionContext{}};

    auto async_get
      { std::async
        ( std::launch::async
        , [&]
          {
            return queue.get
              ( interruption_context
              , milliseconds_from_now (1000)
              );
          }
        )
      };

    queue.error (std::make_exception_ptr (error));

    testing::require_exception
      ( [&]
        {
          std::ignore = async_get.get();
        }
      , testing::Assert<Error>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught, error);
          }
        }
      );
  }

  TEST_F ( IOVBackendDetailQueue
         , can_work_with_move_only_types
         )
  {
    struct MoveOnly
    {
      constexpr explicit MoveOnly (int i) noexcept : _i {i} {}

      MoveOnly() = delete;
      MoveOnly (MoveOnly const&) = delete;
      auto operator= (MoveOnly const&) -> MoveOnly& = delete;
      MoveOnly (MoveOnly&&) noexcept = default;
      auto operator= (MoveOnly&&) noexcept -> MoveOnly& = default;
      ~MoveOnly() noexcept = default;

      int _i;

      [[nodiscard]] constexpr auto operator<=>
        ( MoveOnly const&
        ) const noexcept = default
        ;
    };

    auto queue {Queue<MoveOnly>{}};

    auto const interruption_context {decltype (queue)::InterruptionContext{}};

    auto const get
      { [&]
        {
          return queue.get
            ( interruption_context
            , milliseconds_from_now (1)
            );
        }
      };

    auto const [x0, x1, x2, x3]
      { std::invoke
         ( []
           {
             auto random_int {testing::random::value<int>{}};

             return std::make_tuple
               ( random_int()
               , random_int()
               , random_int()
               , random_int()
               );
           }
         )
      };

    auto m {MoveOnly {x3}};
    queue.push (x0).push (x1).push (MoveOnly {x2}).push (std::move (m));

#define MCS_TEST_IOV_BACKEND_ASSERT_VALUE(_value)       \
    do                                                  \
    {                                                   \
      auto const x {get()};                             \
      ASSERT_EQ (x, MoveOnly {_value});                 \
    }                                                   \
    while (0)

    MCS_TEST_IOV_BACKEND_ASSERT_VALUE (x0);
    MCS_TEST_IOV_BACKEND_ASSERT_VALUE (x1);
    MCS_TEST_IOV_BACKEND_ASSERT_VALUE (x2);
    MCS_TEST_IOV_BACKEND_ASSERT_VALUE (x3);

#undef MCS_TEST_IOV_BACKEND_ASSERT_VALUE
  }

  TEST_F ( IOVBackendDetailQueue
         , multiple_producer_and_consumer_are_possible
         )
  {
    auto random_thread_count {testing::random::value<std::size_t> {1, 10}};
    auto const number_of_producers {random_thread_count()};
    auto const number_of_consumers {random_thread_count()};

    using Element = long;
    auto const elements
      { std::invoke
        ( [&]
          {
            auto _elements {std::vector<Element>{}};

            std::generate_n
              ( std::back_inserter (_elements)
              , testing::random::value<std::size_t> {100, 100000}()
              , testing::random::value<Element>{}
              );

            return _elements;
          }
        )
      };

    auto random_element_position
      {testing::random::value<std::size_t> {0, elements.size()}};

    struct ElementRange
    {
      std::size_t begin;
      std::size_t end;
    };

    auto const element_ranges
      { std::invoke
        ( [&]
          {
            auto _ranges {std::vector<ElementRange>{}};

            std::generate_n
              ( std::back_inserter (_ranges)
              , number_of_producers
              , [&]
                {
                  auto positions
                    { testing::random::unique_values<std::size_t>
                       (random_element_position, 2)
                    };
                  std::ranges::sort (positions);
                  return ElementRange {positions.at (0), positions.at (1)};
                }
              );

            return _ranges;
          }
        )
      };

    struct ThreadResult
    {
      std::size_t number_of_elements {0};
      Element sum_of_elements {0};

      [[nodiscard]] constexpr auto operator+
        ( ThreadResult const& other
        ) const noexcept -> ThreadResult
      {
        return ThreadResult
          { number_of_elements + other.number_of_elements
          , sum_of_elements + other.sum_of_elements
          };
      }

      [[nodiscard]] constexpr auto operator<=>
        ( ThreadResult const&
        ) const noexcept = default
        ;
    };

    auto producers {std::list<std::future<ThreadResult>>{}};
    auto consumers {std::list<std::future<ThreadResult>>{}};

    // producers and consumers are all waiting until all of them are
    // running and only then begin to work, this is meant to maximize
    // parallel work
    //
    auto barrier
      { std::barrier
          { util::cast<std::ptrdiff_t>
              (number_of_producers + number_of_consumers)
          }
      };

    struct GoHome{};

    auto queue {Queue<std::variant<Element, GoHome>>{}};

    // each producer pushes a range of elements into the queue
    //
    std::ranges::transform
      ( element_ranges
      , std::back_inserter (producers)
      , [&] (auto range)
        {
          return std::async
            ( std::launch::async
            , [range, &elements, &barrier, &queue]
              {
                barrier.arrive_and_wait();

                auto result {ThreadResult{}};

                for (auto i {range.begin}; i != range.end; ++i)
                {
                  queue.push (elements.at (i));
                  result.number_of_elements += 1;
                  result.sum_of_elements += elements.at (i);
                }

                return result;
              }
            );
        }
      );

    // each consumer gets entries from the queue until it hits a
    // "GoHome"
    //
    std::generate_n
      ( std::back_inserter (consumers)
      , number_of_consumers
      , [&]
        {
          return std::async
            ( std::launch::async
            , [&barrier, &queue]
              {
                barrier.arrive_and_wait();

                auto result {ThreadResult{}};

                auto const interruption_context
                  {decltype (queue)::InterruptionContext{}};

                auto go_home {false};

                while (!go_home)
                {
                  auto const get_result
                    { queue.get
                        ( interruption_context
                        , milliseconds_from_now (15000)
                        )
                    };

                  std::visit
                    ( util::overloaded
                      { [&] (GoHome) noexcept
                        {
                          go_home = true;
                        }
                      , [&] (Element element) noexcept
                        {
                          result.number_of_elements += 1;
                          result.sum_of_elements += element;
                        }
                      }
                    , get_result
                    );
                }

                return result;
              }
            );
        }
      );

    auto const produced
      { std::accumulate
          ( std::begin (producers), std::end (producers)
          , ThreadResult{}
          , [&] (auto sum, auto& producer)
            {
              return sum + producer.get();
            }
          )
      };

    // all producers are done, tell the consumers they can stop
    //
    std::ranges::for_each
      ( consumers
      , [&] (auto const&)
        {
          queue.push (GoHome{});
        }
      );

    auto const consumed
      { std::accumulate
          ( std::begin (consumers), std::end (consumers)
          , ThreadResult{}
          , [&] (auto sum, auto& consumer)
            {
              return sum + consumer.get();
            }
          )
      };

    ASSERT_EQ (produced, consumed);
  }
}

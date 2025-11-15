// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <barrier>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <map>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/unique_values.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/MapWithHitMissCallbacks.hpp>
#include <mcs/util/cast.hpp>
#include <memory>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace mcs::util
{
  namespace
  {
    struct UtilMapWithHitMissCallbacksR : public testing::random::Test
    {
      testing::random::value<int> random_key{};
      testing::random::value<int> random_value{};
    };

    using Maps = ::testing::Types
      < std::map<int, int>
      , std::unordered_map<int, int>
      >;
    template<class> struct UtilMapWithHitMissCallbacksM
      : public UtilMapWithHitMissCallbacksR
    {};
    TYPED_TEST_SUITE (UtilMapWithHitMissCallbacksM, Maps);
  }

  TYPED_TEST (UtilMapWithHitMissCallbacksM, std_maps_are_associative_map)
  {
    static_assert (is_associative_map<TypeParam, int, int>);
  }

  TYPED_TEST (UtilMapWithHitMissCallbacksM, lambda_can_be_hit_callback)
  {
    using Map = TypeParam;
    auto const lambda {[] (int, Map const&) noexcept{}};

    static_assert (is_hit_callback<decltype (lambda), int, Map>);
  }

  TYPED_TEST ( UtilMapWithHitMissCallbacksM
             , lambda_can_be_miss_callback_and_can_modify_map
             )
  {
    using Map = TypeParam;
    auto const lambda {[] (int, Map&) noexcept{}};

    static_assert (is_miss_callback<decltype (lambda), int, Map>);
  }

  TYPED_TEST (UtilMapWithHitMissCallbacksM, functor_can_be_hit_callback)
  {
    using Map = TypeParam;
    struct Hit
    {
      auto operator() (int, Map const&) const -> void;
    };

    static_assert (is_hit_callback<Hit, int, Map>);
  }

  TYPED_TEST (UtilMapWithHitMissCallbacksM, functor_can_be_miss_callback)
  {
    using Map = TypeParam;
    struct Miss
    {
      auto operator() (int, Map&) const -> void;
    };

    static_assert (is_miss_callback<Miss, int, Map>);
  }

  TYPED_TEST (UtilMapWithHitMissCallbacksM, stateful_functor_can_be_callback)
  {
    using Map = TypeParam;
    struct Callback
    {
      auto hit (int, Map const&) -> void {}
      auto miss (int, Map&) -> void {}
    };

    auto callback {Callback{}};
    auto hit
      { [&] (int i, Map const& m) noexcept
        {
          return callback.hit (i, m);
        }
      };
    auto miss
      { [&] (int i, Map& m) noexcept
        {
          return callback.hit (i, m);
        }
      };

    static_assert (is_hit_callback<decltype (hit), int, Map>);
    static_assert (is_miss_callback<decltype (miss), int, Map>);
  }

  TYPED_TEST ( UtilMapWithHitMissCallbacksM
             , construct_for_non_existing_key_calls_hit_or_miss
             )
  {
    using Map = TypeParam;
    auto map {MapWithHitMissCallbacks<int, int, Map>{}};
    auto miss {0};
    auto hit {0};
    auto const N {testing::random::value<int> {0, 1000}()};
    auto seen {std::unordered_set<int>{}};

    for (auto i {0}; i < N; ++i)
    {
      auto const key {this->random_key()};
      auto const value {this->random_value()};

      if (seen.emplace (key).second)
      {
        ASSERT_EQ
          ( map.at_or_construct
            ( key
            , [&] (auto, auto)
              {
                FAIL() << "unexpected call to hit";
              }
            , [&] (auto k, auto& m)
              {
                ASSERT_EQ (k, key);
                ASSERT_TRUE (!m.contains (k));
                ++miss;
              }
            , value
            )
          , value
          );
      }
      else
      {
        ASSERT_EQ
          ( map.at_or_construct
            ( key
            , [&] (auto k, auto const& m)
              {
                ASSERT_EQ (k, key);
                ASSERT_TRUE (m.contains (k));
                ++hit;
              }
            , [&] (auto, auto&)
              {
                FAIL() << "unexpected call to miss";
              }
            , value
            )
          , value
          );
      }
    }

    ASSERT_EQ (hit + miss, N);
  }

  TYPED_TEST ( UtilMapWithHitMissCallbacksM
             , create_for_non_existing_key_calls_hit_or_miss_and_is_lazy
             )
  {
    using Map = TypeParam;
    auto map {MapWithHitMissCallbacks<int, int, Map>{}};
    auto miss {0};
    auto hit {0};
    auto const N {testing::random::value<int> {0, 1000}()};
    auto seen {std::unordered_set<int>{}};

    for (auto i {0}; i < N; ++i)
    {
      auto const key {this->random_key()};

      if (seen.emplace (key).second)
      {
        map.at_or_create
          ( key
          , [&] (auto, auto)
            {
              FAIL() << "unexpected call to hit";
            }
          , [&] (auto k, auto& m)
              {
                ASSERT_EQ (k, key);
                ASSERT_TRUE (!m.contains (k));
                ++miss;
              }
          , testing::random::value<int>{}
          );
      }
      else
      {
        map.at_or_create
          ( key
          , [&] (auto k, auto const& m)
            {
              ASSERT_EQ (k, key);
              ASSERT_TRUE (m.contains (k));
              ++hit;
            }
          , [&] (auto, auto&)
            {
              FAIL() << "unexpected call to miss";
            }
          , []() -> int
            {
              throw std::logic_error {"Unexpected call to create."};
            }
          );
      }
    }

    ASSERT_EQ (hit + miss, N);
  }

  TYPED_TEST ( UtilMapWithHitMissCallbacksM
             , construct_for_non_existing_key_calls_hit_or_miss_using_a_functor
             )
  {
    using Map = TypeParam;
    auto map {MapWithHitMissCallbacks<int, int, Map>{}};
    struct Callback
    {
      auto operator() (int i, Map const&) noexcept
      {
        ASSERT_FALSE (_seen.emplace (i).second);
        ++_hit;
      }
      auto operator() (int i, Map&) noexcept
      {
        ASSERT_TRUE (_seen.emplace (i).second);
        ++_miss;
      }
      std::unordered_set<int> _seen{};
      int _miss {0};
      int _hit {0};
    };
    auto const N {testing::random::value<int> {0, 1000}()};
    auto callback {Callback{}};

    struct KeyValue
    {
      int key;
      int value;
    };

    auto key_values {std::vector<KeyValue>{}};

    for (auto i {0}; i < N; ++i)
    {
      auto const key {this->random_key()};
      auto const value {this->random_value()};

      ASSERT_EQ
        ( map.at_or_construct
          ( key
          , callback
          , callback
          , value
          )
        , value
        );

      key_values.emplace_back (key, value);
    }

    ASSERT_EQ (callback._hit + callback._miss, N);
    auto const missed {callback._miss};

    for (auto key_value : key_values)
    {
      ASSERT_EQ
        ( map.at_or_construct
          ( key_value.key
          , callback
          , callback
          , this->random_value()
          )
        , key_value.value
        );
    }

    ASSERT_EQ (callback._hit + callback._miss, 2 * N);
    ASSERT_EQ (missed, callback._miss);
  }

  TEST_F (UtilMapWithHitMissCallbacksR, move_only_types_can_be_keys)
  {
    auto map {MapWithHitMissCallbacks<int, std::unique_ptr<int>>{}};

    auto& ptr
      { map.at_or_create
        ( random_key()
        , [] (auto, auto const&) noexcept
          {
            FAIL() << "unexpected call to hit";
          }
        , [] (auto, auto&) noexcept {}
        , std::make_unique<int>
        )
      };

    ASSERT_TRUE (ptr.get() != nullptr);
  }

  TEST_F (UtilMapWithHitMissCallbacksR, unmoveable_types_can_be_keys)
  {
    struct S
    {
      constexpr explicit S (int i_) : i {i_} {}
      S (S const&) = delete;
      S (S&&) = delete;
      auto operator= (S const&) -> S& = delete;
      auto operator= (S&&) -> S& = delete;
      ~S() = default;
      int i;
    };

    auto map {MapWithHitMissCallbacks<int, S>{}};
    auto const value {random_value()};

    ASSERT_EQ
      ( map.at_or_construct
        ( random_key()
        , [] (auto, auto const&) noexcept
          {
            FAIL() << "unexpected call to hit";
          }
        , [] (auto, auto&) noexcept {}
        , value
        ).i
      , value
      );
  }

  TYPED_TEST ( UtilMapWithHitMissCallbacksM
             , multiple_threads_can_use_the_same_callback_misses_only
             )
  {
    using Map = TypeParam;
    struct HitMiss
    {
      HitMiss (int maximum_map_size)
        : _maximum_map_size {maximum_map_size}
      {}

      auto operator() (int, Map const&) noexcept -> void
      {
        FAIL() << "Unexpected call to hit";
      }
      auto operator() (int key, Map& map) noexcept
      {
        ++_miss;

        ASSERT_EQ (map.find (key), std::end (map));

        if (!std::cmp_less (map.size(), _maximum_map_size))
        {
          ASSERT_EQ (map.size(), _maximum_map_size);
          ASSERT_GT (map.size(), 0);
          ASSERT_NE (std::begin (map)->first, key);
          map.erase (std::begin (map));
        }
      }
      int _maximum_map_size;
      int _miss {0};
    };

    using RandomMaximumMapSize = testing::random::value<int>;
    auto const maximum_map_size
      { std::invoke
        ( RandomMaximumMapSize
          { RandomMaximumMapSize::Min {0}
          , RandomMaximumMapSize::Max {1000}
          }
        )
      };

    auto const number_of_threads
      { util::cast<int>
          ( std::max ( std::thread::hardware_concurrency()
                     , 2u
                     )
          )
      };

    auto const ops_per_thread {10 * maximum_map_size};
    auto const keys
      { testing::random::unique_values<int>
          ( this->random_key
          , util::cast<std::size_t> (number_of_threads * ops_per_thread)
          )
      };

    auto hit_miss {HitMiss {maximum_map_size}};
    auto map {ConcurrentMapWithHitMissCallbacks<int, int, Map>{}};

    auto threads {std::vector<std::future<void>>{}};
    auto barrier {std::barrier {number_of_threads}};

    for (auto tid {0}; tid != number_of_threads; ++tid)
    {
      threads.emplace_back
        ( std::async
          ( std::launch::async
          , [&, tid]
            {
              barrier.arrive_and_wait();

              for ( auto key {std::begin (keys) + (tid + 0) * ops_per_thread}
                  ;    key != std::begin (keys) + (tid + 1) * ops_per_thread
                  ; ++key
                  )
              {
                auto const value {this->random_value()};

                ASSERT_EQ
                  ( value
                  , map.at_or_construct
                    ( *key
                    , hit_miss
                    , hit_miss
                    , value
                    )
                  );
              }
            }
          )
        );
    }

    std::ranges::for_each (threads, [] (auto& thread) { thread.get(); });

    ASSERT_EQ (hit_miss._miss, keys.size());
  }

  TYPED_TEST
    ( UtilMapWithHitMissCallbacksM
    , multiple_threads_can_use_the_same_callback_first_insert_misses_only
    )
  {
    using Map = TypeParam;

    struct HitMiss
    {
      HitMiss (int maximum_map_size)
        : _maximum_map_size {maximum_map_size}
      {}

      auto operator() (int key, Map const& map) noexcept -> void
      {
        ++_hit;

        ASSERT_NE (map.find (key), std::end (map));
        ASSERT_LE (map.size(), _maximum_map_size);
      }
      auto operator() (int key, Map& map) noexcept
      {
        ++_miss;

        ASSERT_EQ (map.find (key), std::end (map));

        if (!std::cmp_less (map.size(), _maximum_map_size))
        {
          FAIL() << "Unexpected eviction request";
        }
      }
      int _maximum_map_size;
      int _hit {0};
      int _miss {0};
    };

    using RandomMaximumMapSize = testing::random::value<int>;
    auto const maximum_map_size
      { std::invoke
        ( RandomMaximumMapSize
          { RandomMaximumMapSize::Min {1}
          , RandomMaximumMapSize::Max {1000}
          }
        )
      };

    auto const number_of_threads
      { util::cast<int>
          ( std::max ( std::thread::hardware_concurrency()
                     , 2u
                     )
          )
      };

    auto const ops_per_thread {10 * maximum_map_size};

    auto hit_miss {HitMiss {maximum_map_size}};
    auto map {ConcurrentMapWithHitMissCallbacks<int, int, Map>{}};

    auto threads {std::vector<std::future<void>>{}};
    auto barrier {std::barrier {number_of_threads}};

    for (auto tid {0}; tid != number_of_threads; ++tid)
    {
      threads.emplace_back
        ( std::async
          ( std::launch::async
          , [&]
            {
              barrier.arrive_and_wait();

              using RandomKey = testing::random::value<int>;
              auto small_random_key
                { RandomKey
                  { RandomKey::Min {0}
                  , RandomKey::Max {maximum_map_size - 1}
                  }
                };

              for (auto i {0}; i != ops_per_thread; ++i)
              {
                // nothing to test: the key might be present already,
                // in which case the earlier inserted value is returned
                std::ignore = map.at_or_construct
                  ( small_random_key()
                  , hit_miss
                  , hit_miss
                  , this->random_value()
                  );
              }
            }
          )
        );
    }

    std::ranges::for_each (threads, [] (auto& thread) { thread.get(); });

    ASSERT_LE ( hit_miss._miss
              , maximum_map_size
              );
    ASSERT_EQ ( hit_miss._hit + hit_miss._miss
              , number_of_threads * ops_per_thread
              );
  }
}

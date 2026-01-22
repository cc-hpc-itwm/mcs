// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/nonstd/scope.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/HeterogeneousMap.hpp>
#include <numeric>
#include <string>
#include <thread>
#include <tuple>
#include <variant>
#include <vector>

namespace
{
  struct Identity
  {
    auto operator() (auto const& x) noexcept
    {
      return x;
    }
  };

  struct ConstZero
  {
    auto operator() (auto const&) noexcept
    {
      return 0;
    }
  };
}

namespace mcs::util
{
  struct UnsynchronizedHeterogeneousMapR : public testing::random::Test{};

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , empty_map_throws_when_accessing_any_key_via_at
         )
  {
    using Key = int;
    using Value = std::string;

    auto hmap {UnsynchronizedHeterogeneousMap<Key, util::type::List<Value>>{}};
    auto const key {testing::random::value<Key>{}()};

    testing::require_exception
      ( [&]
        {
          std::ignore = hmap.at (key);
        }
      , testing::assert_type_and_what<decltype (hmap)::Error::UnknownKey>
          ( fmt::format ("Unknown key '{}'", key)
          )
      );
  }

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , empty_map_throws_when_accessing_any_key_via_visit
         )
  {
    using Key = int;
    using Value = std::string;

    auto hmap {UnsynchronizedHeterogeneousMap<Key, util::type::List<Value>>{}};
    auto const key {testing::random::value<Key>{}()};

    testing::require_exception
      ( [&]
        {
          std::ignore = hmap.visit (key, ConstZero{});
        }
      , testing::assert_type_and_what<decltype (hmap)::Error::UnknownKey>
          ( fmt::format ("Unknown key '{}'", key)
          )
      );
  }

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , empty_map_throws_when_accessing_any_key_via_invoke
         )
  {
    using Key = int;
    using Value = std::string;

    auto hmap {UnsynchronizedHeterogeneousMap<Key, util::type::List<Value>>{}};
    auto const key {testing::random::value<Key>{}()};

    testing::require_exception
      ( [&]
        {
          std::ignore = hmap.visit (key, Identity{});
        }
      , testing::assert_type_and_what<decltype (hmap)::Error::UnknownKey>
          ( fmt::format ("Unknown key '{}'", key)
          )
      );
  }

  TEST_F (UnsynchronizedHeterogeneousMapR, to_remove_unknown_key_works)
  {
    using Key = int;
    using Value = std::string;

    auto hmap {UnsynchronizedHeterogeneousMap<Key, util::type::List<Value>>{}};

    hmap.remove (testing::random::value<Key>{}());

    ASSERT_TRUE (true);
  }

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , to_access_removed_key_via_at_throws
         )
  {
    using Key = int;
    using Value = long;

    auto hmap {UnsynchronizedHeterogeneousMap<Key, util::type::List<Value>>{}};
    auto const key {hmap.create<Value> (testing::random::value<Value>{}())};
    hmap.remove (key);

    testing::require_exception
      ( [&]
        {
          std::ignore = hmap.at (key);
        }
      , testing::assert_type_and_what<decltype (hmap)::Error::UnknownKey>
          ( fmt::format ("Unknown key '{}'", key)
          )
      );
  }
  TEST_F ( UnsynchronizedHeterogeneousMapR
         , to_access_removed_key_via_visit_throws
         )
  {
    using Key = int;
    using Value = long;

    auto hmap {UnsynchronizedHeterogeneousMap<Key, util::type::List<Value>>{}};
    auto const key {hmap.create<Value> (testing::random::value<Value>{}())};
    hmap.remove (key);

    testing::require_exception
      ( [&]
        {
          std::ignore = hmap.visit (key, ConstZero{});
        }
      , testing::assert_type_and_what<decltype (hmap)::Error::UnknownKey>
          ( fmt::format ("Unknown key '{}'", key)
          )
      );
  }
  TEST_F ( UnsynchronizedHeterogeneousMapR
         , to_access_removed_key_via_invoke_throws
         )
  {
    using Key = int;
    using Value = long;

    auto hmap {UnsynchronizedHeterogeneousMap<Key, util::type::List<Value>>{}};
    auto const key {hmap.create<Value> (testing::random::value<Value>{}())};
    hmap.remove (key);

    testing::require_exception
      ( [&]
        {
          std::ignore = hmap.visit (key, Identity{});
        }
      , testing::assert_type_and_what<decltype (hmap)::Error::UnknownKey>
          ( fmt::format ("Unknown key '{}'", key)
          )
      );
  }

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , to_access_inserted_key_via_at_returns_variant_with_stored_value
         )
  {
    using Key = int;
    using Value = long;

    auto hmap {UnsynchronizedHeterogeneousMap<Key, util::type::List<Value>>{}};
    auto const value {testing::random::value<Value>{}()};
    auto const key {hmap.create<Value> (value)};

    ASSERT_EQ (hmap.at (key), std::variant<Value> {value});
  }

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , to_access_inserted_key_via_invoke_returns_stored_value
         )
  {
    using Key = int;

    auto hmap
      { UnsynchronizedHeterogeneousMap
          < Key
          , util::type::List<int, std::string>
          >{}
      };

    auto const i {testing::random::value<int>{}()};
    auto const s {testing::random::value<std::string>{}()};

    auto const key_i {hmap.create<int> (i)};
    auto const key_s {hmap.create<std::string> (s)};

    ASSERT_EQ (i, hmap.template invoke<int> (key_i, Identity{}));
    ASSERT_EQ (s, hmap.template invoke<std::string> (key_s, Identity{}));
  }

  namespace
  {
    struct EqualTo
    {
      template<typename T>
        [[nodiscard]] auto operator() (T const& x, T const& y) const -> bool
      {
        return x == y;
      }
      template<typename T, typename U>
        [[nodiscard]] auto operator() (T const&, U const&) const -> bool
      {
        throw std::runtime_error {"type mismatch"};
      }
    };

    struct USHMapLS
      : public UnsynchronizedHeterogeneousMap<int, util::type::List<long, std::string>>
    {
      using Base = UnsynchronizedHeterogeneousMap<int, util::type::List<long, std::string>>;

      template<typename T>
        [[nodiscard]] auto create (T x)
      {
        return Base::template create<T> (x);
      }

      template<typename T>
        [[nodiscard]] auto equal (int key, T const& expected) const
      {
        return Base::visit
          ( key
          , [&] (auto const& x)
            {
              return EqualTo{} (x, expected);
            }
          );
      }
    };
  }

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , to_access_the_correct_out_of_multiple_types_works
         )
  {
    auto hmap {USHMapLS{}};
    auto const l {testing::random::value<long>{}()};
    auto const s {testing::random::value<std::string>{}()};
    auto const key_l {hmap.create (l)};
    auto const key_s {hmap.create (s)};

    ASSERT_TRUE (hmap.equal (key_l, l));
    ASSERT_TRUE (hmap.equal (key_s, s));
  }

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , to_access_the_wrong_out_of_multiple_types_throws
         )
  {
    auto hmap {USHMapLS{}};
    auto const key {hmap.create (testing::random::value<long>{}())};

    testing::require_exception
      ( [&]
        {
          std::ignore
            = hmap.equal (key, testing::random::value<std::string>{}())
            ;
        }
      , testing::assert_type_and_what<std::runtime_error> ("type mismatch")
      );
  }

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , intended_use_as_base_for_polymorphic_container_works
         )
  {
    struct A
    {
      int _value;
      [[nodiscard]] auto f() const noexcept -> int
      {
        return _value;
      }
    };
    struct B
    {
      int _value;
      [[nodiscard]] auto f() const noexcept -> int
      {
        return _value;
      }
    };

    struct ABMap : public UnsynchronizedHeterogeneousMap<int, util::type::List<A, B>>
    {
      using Base = UnsynchronizedHeterogeneousMap<int, util::type::List<A, B>>;

      [[nodiscard]] auto f (int key) const
      {
        return Base::visit
          ( key
          , [] (auto const& a_or_b) noexcept
            {
              return a_or_b.f();
            }
          );
      }
    };

    auto ab_map {ABMap{}};

    auto random_int {testing::random::value<int>{}};
    auto const va {random_int()};
    auto const vb {random_int()};
    auto const [key_A, key_B]
      { std::invoke
        ( [&]
          {
            return std::make_tuple
              ( ab_map.template create<A> (va)
              , ab_map.template create<B> (vb)
              );
          }
        )
      };

    ASSERT_EQ (ab_map.f (key_A), va);
    ASSERT_EQ (ab_map.f (key_B), vb);
  }

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , visit_can_be_called_directly_and_visits_the_contained_object
         )
  {
    struct A
    {
      int _value;
      [[nodiscard]] auto f() const noexcept -> int
      {
        return _value;
      }
    };
    struct B
    {
      int _value;
      [[nodiscard]] auto f() const noexcept -> int
      {
        return _value;
      }
    };

    auto ab_map {UnsynchronizedHeterogeneousMap<int, util::type::List<A, B>>{}};
    auto random_int {testing::random::value<int>{}};
    auto const va {random_int()};
    auto const vb {random_int()};
    auto const [key_A, key_B]
      { std::invoke
        ( [&]
          {
            return std::make_tuple
              ( ab_map.template create<A> (va)
              , ab_map.template create<B> (vb)
              );
          }
        )
      };

    ab_map.visit
      ( key_A
      , [&] (auto const& x)
        {
          ASSERT_EQ (typeid (x), typeid (A));
          ASSERT_EQ (x._value, va);
        }
      );
    ab_map.visit
      ( key_B
      , [&] (auto const& x)
        {
          ASSERT_EQ (typeid (x), typeid (B));
          ASSERT_EQ (x._value, vb);
        }
      );
  }

  TEST_F (UnsynchronizedHeterogeneousMapR, visit_documentation_example_works)
  {
    auto hmap {UnsynchronizedHeterogeneousMap<int, util::type::List<int, std::string>>{}};
    auto const value {std::invoke (testing::random::value<int>{})};
    auto const key {hmap.template create<int> (value)};
    hmap.visit
      ( key
      , [&] (auto const& x)
        {
          ASSERT_EQ (typeid (x), typeid (int));
          ASSERT_TRUE (EqualTo{} (x, value));
        }
      );
  }

  TEST_F ( UnsynchronizedHeterogeneousMapR
         , contained_types_must_neither_be_copyable_nor_moveable
         )
  {
    struct A
    {
      A (int v) : _v {v} {}

      [[nodiscard]] auto f() const noexcept -> int { return _v; }

      A (A const&) = delete;
      A (A&&) = delete;
      auto operator= (A const&) -> A& = delete;
      auto operator= (A&&) -> A& = delete;
      ~A() = default;

    private:
      int _v;
    };
    struct B
    {
      B (int v) : _v {v} {}

      [[nodiscard]] auto f() const noexcept -> int { return _v; }

      B (B const&) = delete;
      B (B&&) = delete;
      auto operator= (B const&) -> B& = delete;
      auto operator= (B&&) -> B& = delete;
      ~B() = default;

    private:
      int _v;
    };

    struct ABMap : public UnsynchronizedHeterogeneousMap<int, util::type::List<A, B>>
    {
      using Base = UnsynchronizedHeterogeneousMap<int, util::type::List<A, B>>;

      [[nodiscard]] auto f (int key) const
      {
        return Base::visit
          ( key
          , [] (auto const& a_or_b) noexcept
            {
              return a_or_b.f();
            }
          );
      }
    };

    auto ab_map {ABMap{}};
    auto const v_a {testing::random::value<int>{}()};
    auto const v_b {testing::random::value<int>{}()};

    auto const [key_A, key_B]
      { std::invoke
        ( [&]
          {
            return std::make_tuple
              ( ab_map.template create<A> (v_a)
              , ab_map.template create<B> (v_b)
              );
          }
        )
      };

    ASSERT_EQ (ab_map.f (key_A), v_a);
    ASSERT_EQ (ab_map.f (key_B), v_b);
  }
}

namespace mcs::util
{
  struct HeterogeneousMapR : public testing::random::Test{};

  TEST_F ( HeterogeneousMapR
         , read_access_and_read_write_access_can_be_stored_for_bulk_operation
         )
  {
    using Key = int;
    using Value = long;

    auto const values
      { std::invoke
        ( []
          {
            auto const k {testing::random::value<unsigned> {0U, 1000U}()};

            auto _values {std::vector<Value>{}};
            _values.reserve (k);

            std::generate_n
              ( std::back_inserter (_values)
              , k
              , testing::random::value<Value>{}
              );

            return _values;
          }
        )
      };

    auto hmap {HeterogeneousMap<Key, util::type::List<Value>>{}};

    auto const keys
      { std::invoke
        ( [&]
          {
            auto _keys {std::vector<Key>{}};
            _keys.reserve (values.size());

            auto const read_write_access {hmap.read_write_access()};

            std::ranges::transform
              ( values
              , std::back_inserter (_keys)
              , [&] (auto const& value)
                {
                  return read_write_access.template create<Value> (value);
                }
              );

            return _keys;
          }
        )
      };

    {
      auto expected {std::begin (values)};

      auto const read_access {hmap.read_access()};

      for (auto key : keys)
      {
        ASSERT_EQ
          ( *expected++
          , read_access.template invoke<Value> (key, Identity{})
          );
      }
    }

    {
      auto const read_write_access {hmap.read_write_access()};

      std::ranges::for_each
        ( keys
        , [&] (auto const& key)
          {
            return read_write_access.remove (key);
          }
        );
    }

    {
      auto const read_access {hmap.read_access()};

      for (auto key : keys)
      {
        testing::require_exception
          ( [&]
            {
              std::ignore = read_access.visit (key, ConstZero{});
            }
          , testing::assert_type_and_what<decltype (hmap)::Error::UnknownKey>
              ( fmt::format ("Unknown key '{}'", key)
              )
          );
      }
    }
  }

  namespace
  {
    template<typename HMap>
      auto multiple_reads_one_write() -> long
    {
      auto hmap {HMap{}};
      auto key {hmap.create (hmap.read_write_access(), 0L)};
      //   ^ modified by writer!

      auto const execution_time {std::chrono::milliseconds {300}};
      auto const create_delay {std::chrono::milliseconds {100}};
      auto const number_of_readers {30};

      auto const reader
        { [&]()
          {
            return std::async
              ( std::launch::async
              , [&]
                {
                  auto const read_access {hmap.read_access()};

                  std::this_thread::sleep_for (execution_time);

                  return hmap.at (read_access, key);
                }
              );
          }
        };

      auto readers {std::vector<std::future<long>>{}};

      auto reader_generator
        { std::async
          ( std::launch::async
          , [&]
            {
              for (auto r {0}; r != number_of_readers; ++r)
              {
                readers.emplace_back (reader());

                std::this_thread::sleep_for (create_delay);
              }
            }
          )
        };

      std::this_thread::sleep_for (create_delay);

      // writer
      {
        auto const read_write_access {hmap.read_write_access()};

        std::this_thread::sleep_for (execution_time);

        key = hmap.create (read_write_access, 1L);
      }

      reader_generator.wait();

      return std::accumulate
        ( std::begin (readers), std::end (readers)
        , 0L
        , [] (auto s, auto& r)
          {
            return s + r.get();
          }
        );
    }
  }

  TEST (HeterogeneousMap, write_has_priority_over_reads)
  {
    using Key = int;
    using Value = long;

    struct HMap : public HeterogeneousMap<Key, util::type::List<Value>>
    {
      using Base = HeterogeneousMap<Key, util::type::List<Value>>;

      [[nodiscard]] auto create
        ( Base::ReadWriteAccess const& read_write_access
        , Value value
        ) const -> Key
      {
        return read_write_access.template create<Value> (value);
      }

      [[nodiscard]] auto at
        ( typename HMap::ReadAccess const& read_access
        , Key key
        ) const
      {
        return read_access.template invoke<Value> (key, Identity{});
      }
    };

    ASSERT_GT (multiple_reads_one_write<HMap>(), 0L);
  }
}

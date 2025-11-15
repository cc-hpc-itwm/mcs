// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <atomic>
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
#include <variant>
#include <vector>

namespace mcs::util
{
  struct HeterogeneousMapR : public testing::random::Test{};

  TEST_F (HeterogeneousMapR, empty_map_throws_when_accessing_any_id)
  {
    using ID = int;
    using Value = std::string;

    struct HMap : public HeterogeneousMap<ID, Value>
    {
      [[nodiscard]] auto at (ID id) const
      {
        return visit
          ( read_access()
          , id
          , [] (Value v) { FAIL() << "unexpected value " << v; }
          );
      }
    };
    auto hmap {HMap{}};
    auto const id {testing::random::value<ID>{}()};

    testing::require_exception
      ( [&]
        {
          hmap.at (id);
        }
      , testing::assert_type_and_what<decltype (hmap)::Error::UnknownID>
          ( fmt::format ("Unknown id '{}'", id)
          )
      );
  }

  TEST_F (HeterogeneousMapR, to_remove_unknown_id_works)
  {
    using ID = int;
    using Value = std::string;

    auto hmap {HeterogeneousMap<ID, Value>{}};

    hmap.remove (hmap.write_access(), testing::random::value<ID>{}());

    ASSERT_TRUE (true);
  }

  TEST_F (HeterogeneousMapR, to_access_inserted_id_returns_stored_value)
  {
    using ID = int;
    using Value = long;

    struct HMap : public HeterogeneousMap<ID, Value>
    {
      using HeterogeneousMap::at;

      [[nodiscard]] auto at (ID id) const
      {
        return visit
          ( read_access()
          , id
          , [] (Value v) noexcept { return v; }
          );
      }
    };
    auto hmap {HMap{}};
    auto const value {testing::random::value<Value>{}()};
    auto const id {hmap.create<Value> (hmap.write_access(), value)};

    ASSERT_EQ (hmap.at (id), value);
  }

  TEST_F (HeterogeneousMapR, to_access_removed_id_throws)
  {
    using ID = int;
    using Value = long;

    struct HMap : public HeterogeneousMap<ID, Value>
    {
      [[nodiscard]] auto at (ID id) const
      {
        return visit
          ( read_access()
          , id
          , [] (Value v) { FAIL() << "unexpected value " << v; }
          );
      }
    };
    auto hmap {HMap{}};
    auto const id
      { hmap.create<Value>
          (hmap.write_access(), testing::random::value<Value>{}())
      };
    hmap.remove (hmap.write_access(), id);

    testing::require_exception
      ( [&]
        {
          hmap.at (id);
        }
      , testing::assert_type_and_what<decltype (hmap)::Error::UnknownID>
          ( fmt::format ("Unknown id '{}'", id)
          )
      );
  }

  TEST_F ( HeterogeneousMapR
         , to_access_inserted_id_via_at_returns_variant_of_stored_value
         )
  {
    using ID = int;

    auto hmap {HeterogeneousMap<ID, int, std::string>{}};

    auto const i {testing::random::value<int>{}()};
    auto const s {testing::random::value<std::string>{}()};

    auto const id_i {hmap.create<int> (hmap.write_access(), i)};
    auto const id_s {hmap.create<std::string> (hmap.write_access(), s)};

    using Variant = std::variant<int, std::string>;

    ASSERT_EQ (hmap.at (hmap.read_access(), id_i), Variant {i});
    ASSERT_EQ (hmap.at (hmap.read_access(), id_s), Variant {s});
  }

  TEST_F (HeterogeneousMapR, to_access_removed_id_via_at_throws)
  {
    using ID = int;
    using Value = long;

    auto hmap {HeterogeneousMap<ID, Value>{}};

    auto const id
      { hmap.create<Value>
          (hmap.write_access(), testing::random::value<Value>{}())
      };
    hmap.remove (hmap.write_access(), id);

    testing::require_exception
      ( [&]
        {
          std::ignore = hmap.at (hmap.read_access(), id);
        }
      , testing::assert_type_and_what<decltype (hmap)::Error::UnknownID>
          ( fmt::format ("Unknown id '{}'", id)
          )
      );
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

    struct HMapLS : public HeterogeneousMap<int, long, std::string>
    {
      using Base = HeterogeneousMap<int, long, std::string>;

      template<typename T>
        [[nodiscard]] auto create (T value) -> int
      {
        return Base::create<T> (write_access(), value);
      }

      template<typename T>
        [[nodiscard]] auto equal (int id, T const& expected) const
      {
        return visit
          ( read_access()
          , id
          , [&] (auto const& x)
            {
              return EqualTo{} (x, expected);
            }
          );
      }
    };
  }

  TEST_F (HeterogeneousMapR, to_access_the_correct_out_of_multiple_types_works)
  {
    auto hmap {HMapLS{}};
    auto const l {testing::random::value<long>{}()};
    auto const s {testing::random::value<std::string>{}()};
    auto const id_l {hmap.create (l)};
    auto const id_s {hmap.create (s)};

    ASSERT_TRUE (hmap.equal (id_l, l));
    ASSERT_TRUE (hmap.equal (id_s, s));
  }

  TEST_F ( HeterogeneousMapR
         , to_access_the_wrong_out_of_multiple_types_is_possible
         )
  {
    auto hmap {HMapLS{}};
    auto const id {hmap.create (testing::random::value<long>{}())};

    testing::require_exception
      ( [&]
        {
          std::ignore
            = hmap.equal (id, testing::random::value<std::string>{}())
            ;
        }
        , testing::assert_type_and_what<std::runtime_error> ("type mismatch")
      );
  }

  TEST_F ( HeterogeneousMapR
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

    struct ABMap : public HeterogeneousMap<int, A, B>
    {
      [[nodiscard]] auto f (int id) const
      {
        return visit
          ( read_access()
          , id
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
    auto const id_A {ab_map.create<A> (ab_map.write_access(), va)};
    auto const id_B {ab_map.create<B> (ab_map.write_access(), vb)};

    ASSERT_EQ (ab_map.f (id_A), va);
    ASSERT_EQ (ab_map.f (id_B), vb);
  }

  TEST_F ( HeterogeneousMapR
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

    auto ab_map {HeterogeneousMap<int, A, B>{}};
    auto random_int {testing::random::value<int>{}};
    auto const va {random_int()};
    auto const vb {random_int()};
    auto const id_A {ab_map.create<A> (ab_map.write_access(), va)};
    auto const id_B {ab_map.create<B> (ab_map.write_access(), vb)};

    ab_map.visit
      ( ab_map.read_access()
      , id_A
      , [&] (auto const& x)
        {
          ASSERT_EQ (typeid (x), typeid (A));
          ASSERT_EQ (x._value, va);
        }
      );
    ab_map.visit
      ( ab_map.read_access()
      , id_B
      , [&] (auto const& x)
        {
          ASSERT_EQ (typeid (x), typeid (B));
          ASSERT_EQ (x._value, vb);
        }
      );
  }

  TEST_F (HeterogeneousMapR, visit_documentation_example_works)
  {
    auto hmap {HeterogeneousMap<int, int, std::string>{}};
    auto const value {std::invoke (testing::random::value<int>{})};
    auto const id {hmap.create<int> (hmap.write_access(), value)};
    hmap.visit
      ( hmap.read_access()
      , id
      , [&] (auto const& x)
        {
          ASSERT_EQ (typeid (x), typeid (int));
          ASSERT_TRUE (EqualTo{} (x, value));
        }
      );
  }

  TEST_F ( HeterogeneousMapR
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

    struct ABMap : public HeterogeneousMap<int, A, B>
    {
      [[nodiscard]] auto f (int id) const
      {
        return visit
          ( read_access()
          , id
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

    auto const id_A {ab_map.create<A> (ab_map.write_access(), v_a)};
    auto const id_B {ab_map.create<B> (ab_map.write_access(), v_b)};

    ASSERT_EQ (ab_map.f (id_A), v_a);
    ASSERT_EQ (ab_map.f (id_B), v_b);
  }

  TEST_F ( HeterogeneousMapR
         , read_and_write_access_can_be_stored_for_bulk_operation
         )
  {
    using ID = int;
    using Value = long;

    struct HMap : public HeterogeneousMap<ID, Value>
    {
      [[nodiscard]] auto at (ReadAccess const& read_access, ID id) const
      {
        return visit
          ( read_access
          , id
          , [] (Value v) noexcept { return v; }
          );
      }
    };
    auto hmap {HMap{}};

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
    auto const ids
      { std::invoke
        ( [&]
          {
            auto _ids {std::vector<ID>{}};
            _ids.reserve (values.size());

            auto const write_access {hmap.write_access()};

            std::ranges::transform
              ( values
              , std::back_inserter (_ids)
              , [&] (auto const& value)
                {
                  return hmap.create<Value> (write_access, value);
                }
              );

            return _ids;
          }
        )
      };

    {
      auto expected {std::begin (values)};

      auto const read_access {hmap.read_access()};

      for (auto id : ids)
      {
        ASSERT_EQ (*expected++, hmap.at (read_access, id));
      }
    }

    {
      auto const write_access {hmap.write_access()};

      std::ranges::for_each
        ( ids
        , [&] (auto const& id)
          {
            return hmap.remove (write_access, id);
          }
        );
    }

    {
      auto const read_access {hmap.read_access()};

      for (auto id : ids)
      {
        testing::require_exception
          ( [&]
            {
              std::ignore = hmap.at (read_access, id);
            }
          , testing::assert_type_and_what<decltype (hmap)::Error::UnknownID>
              ( fmt::format ("Unknown id '{}'", id)
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
      auto id {hmap.create (hmap.write_access(), 0L)};
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

                  return hmap.at (read_access, id);
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
        auto const write_access {hmap.write_access()};

        std::this_thread::sleep_for (execution_time);

        id = hmap.create (write_access, 1L);
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

  TEST (HeterogeneousMap, write_can_be_delayed_by_a_chain_of_reads_for_ever)
  {
    using ID = int;
    using Value = long;

    struct HMap : public HeterogeneousMap<ID, Value>
    {
      using Base =  HeterogeneousMap<ID, Value>;
      using Base::read_access;
      using Base::write_access;

      [[nodiscard]] auto create
        ( WriteAccess const& write_access
        , Value value
        ) -> ID
      {
        return Base::create<Value> (write_access, value);
      }

      [[nodiscard]] auto at (ReadAccess const& read_access, ID id) const
      {
        return visit
          ( read_access
          , id
          , [] (Value v) noexcept { return v; }
          );
      }
    };

    ASSERT_EQ (multiple_reads_one_write<HMap>(), 0L);
  }

  TEST (HeterogeneousMap, write_can_get_priority_using_an_atomic)
  {
    using ID = int;
    using Value = long;

    struct HMap : public HeterogeneousMap<ID, Value>
    {
      using Base =  HeterogeneousMap<ID, Value>;

      [[nodiscard]] auto read_access() -> ReadAccess
      {
        while (_number_of_waiting_writers.load() != 0)
        {
          std::this_thread::yield();
        }

        return Base::read_access();
      }

      [[nodiscard]] auto write_access() -> WriteAccess
      {
        ++_number_of_waiting_writers;

        return Base::write_access();
      }

      [[nodiscard]] auto create
        ( WriteAccess const& write_access
        , Value value
        ) -> ID
      {
        auto const decrement_number_of_writers
          { nonstd::make_scope_exit
            ( [&]() noexcept
              {
                --_number_of_waiting_writers;
              }
            )
          };

        return Base::create<Value> (write_access, value);
      }

      [[nodiscard]] auto at (ReadAccess const& read_access, ID id) const
      {
        return visit
          ( read_access
          , id
          , [] (Value v) noexcept { return v; }
          );
      }

    private:
      std::atomic<std::size_t> _number_of_waiting_writers {0};
    };

    ASSERT_GT (multiple_reads_one_write<HMap>(), 0L);
  }

  TEST_F (HeterogeneousMapR, using_an_access_token_from_another_object_throws)
  {
    using ID = int;
    using Value = std::string;

    struct HMap : public HeterogeneousMap<ID, Value>{};
    auto hmap1 {HMap{}};
    auto hmap2 {HMap{}};

    testing::require_exception
      ( [&]
        {
          std::ignore = hmap2.create<Value>
            ( hmap1.write_access()
            , testing::random::value<Value>{}()
            );
        }
      , testing::assert_type_and_what<decltype (hmap2)::Error::AccessTokenDoesNotBelongToThis>
          ( "Access token does not belong to this."
          )
      );
  }
}

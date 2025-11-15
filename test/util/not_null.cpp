// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>

namespace mcs::util
{
  TEST (UtilNotNull, is_not_default_constructible)
  {
    static_assert (!std::is_default_constructible_v<not_null<int>>);
  }
  TEST (UtilNotNull, is_copy_assignable)
  {
    static_assert (std::is_copy_assignable_v<not_null<int>>);
  }
  TEST (UtilNotNull, is_move_assignable)
  {
    static_assert (std::is_move_assignable_v<not_null<int>>);
  }
  TEST (UtilNotNull, is_copy_constructable)
  {
    static_assert (std::is_copy_constructible_v<not_null<int>>);
  }
  TEST (UtilNotNull, is_move_constructable)
  {
    static_assert (std::is_move_constructible_v<not_null<int>>);
  }

  TEST (UtilNotNull, construction_from_nullptr_throws)
  {
    using NN = not_null<int>;

    testing::require_exception
      ( []
        {
          std::ignore = NN {nullptr};
        }
     , testing::assert_type_and_what<typename NN::Error::MustNotBeNull>
        ( "not_null: must not be null"
        )
     );
  }
  TEST (UtilNotNull, operator_arrow_allows_to_read_members)
  {
    struct S { int m; };
    auto const value {testing::random::value<int>{}()};
    auto const s {S {value}};
    auto const nn {not_null {std::addressof (s)}};
    static_assert (std::is_same_v<decltype (nn->m), int>);
    ASSERT_EQ (nn->m, value);
  }
  TEST (UtilNotNull, operator_arrow_allows_to_modify_members)
  {
    struct S { int m; };
    auto const value1 {testing::random::value<int>{}()};
    auto const value2 {testing::random::value<int>{}()};
    auto s {S {value1}};
    auto const nn {not_null {std::addressof (s)}};
    nn->m = value2;
    ASSERT_EQ (s.m, value2);
  }
  TEST (UtilNotNull, operator_arrow_of_nn_const_allows_to_read_members)
  {
    struct S { int m; };
    auto const value {testing::random::value<int>{}()};
    auto const s {S {value}};
    auto const nn {not_null<S const> {std::addressof (s)}};
    static_assert (std::is_same_v<decltype (nn->m), int>);
    ASSERT_EQ (nn->m, value);
  }

  TEST (UtilNotNull, operator_star_returns_const_reference)
  {
    struct S { int m; };
    auto const value {testing::random::value<int>{}()};
    auto const s {S {value}};
    auto const nn {not_null {std::addressof (s)}};
    static_assert (std::is_same_v<decltype (*nn), S const&>);
    ASSERT_EQ ((*nn).m, value);
  }
}

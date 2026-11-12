// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/read/ParseError.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/type/List.hpp>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace mcs::util::type
{
  namespace
  {
    template<typename T>
      struct ParsedType
    {
      using type = T;
    };

    struct ParsedRunner
    {
      template<typename T>
        [[nodiscard]] auto operator()
          ( T value
          , int increment
          ) const noexcept -> T
      {
        static_assert (std::is_same_v<T, int>);
        return value + increment;
      }
    };
  }

  TEST (UtilTypesList, parse_id_and_run_accepts_strings_and_bounded_views)
  {
    using Types = List<int>;

    EXPECT_EQ
      ( Types::parse_id_and_run<ParsedType> ("42", ParsedRunner{}, 1)
      , 43
      );

    auto const input {std::string {"42"}};
    EXPECT_EQ
      ( Types::parse_id_and_run<ParsedType> (input, ParsedRunner{}, 1)
      , 43
      );
    EXPECT_EQ
      ( Types::parse_id_and_run<ParsedType>
          (std::string {"42"}, ParsedRunner{}, 1)
      , 43
      );
    EXPECT_EQ
      ( Types::parse_id_and_run<ParsedType>
          (std::string_view {input.data(), 1UL}, ParsedRunner{}, 1)
      , 5
      );
  }

  TEST (UtilTypesList, parse_id_and_run_reports_bounded_input_errors)
  {
    using Types = List<int>;

    auto const input {std::string {"x42"}};

    testing::require_exception
      ( [&]
        {
          std::ignore = Types::parse_id_and_run<ParsedType>
            ( std::string_view {input.data(), 1UL}
            , ParsedRunner{}
            , 0
            );
        }
      , testing::assert_type_and_what<read::ParseError>
          ("ParseError:\nx\n^\n: Not any of the alternatives")
      );

    testing::require_exception
      ( [&]
        {
          std::ignore = Types::parse_id_and_run<ParsedType>
            ( std::string_view{}
            , ParsedRunner{}
            , 0
            );
        }
      , testing::assert_type_and_what<read::ParseError>
          ("ParseError:\n\n^\n: Not any of the alternatives")
      );
  }

  TEST (UtilTypesList, contains_returns_true_for_all_types_that_are_contained)
  {
    struct A;
    using Ts = List<int, A>;

    static_assert (Ts::contains<int>());
    static_assert (Ts::contains<A>());

  }

  TEST (UtilTypesList, contains_returns_false_for_types_that_are_not_contained)
  {
    struct A;
    using Ts = List<int, A>;

    static_assert (!Ts::contains<char>());
  }

  TEST (UtilTypesList, run_of_id_calls_runner_with_correct_type_and_args)
  {
    struct A{};
    using Ts = List<int, A>;

    auto const v {testing::random::value<long>{}()};

    Ts::run
      ( Ts::id<int>()
      , [&]<typename T> (long x)
        {
          ASSERT_EQ (typeid (T), typeid (int));
          ASSERT_NE (typeid (T), typeid (A));
          ASSERT_EQ (x, v);
        }
      , v
      );

    Ts::run
      ( Ts::id<A>()
      , [&]<typename T>()
        {
          ASSERT_NE (typeid (T), typeid (int));
          ASSERT_EQ (typeid (T), typeid (A));
        }
      );
  }

  TEST (UtilTypesList, variant_type_contains_all_types_of_the_list)
  {
    struct A{};
    using Ts = List<int, A>;
    auto const i {Ts::Variant {42}};
    ASSERT_TRUE (std::holds_alternative<int> (i));
    auto const a {Ts::Variant {std::in_place_type_t<A>{}}};
    ASSERT_TRUE (std::holds_alternative<A> (a));
  }

  namespace
  {
    template<typename> struct Dep;
  }

  TEST (UtilTypesList, fmap_creates_list_of_dependent_types)
  {
    using Ts = List<int, long>;
    using Ds = typename Ts::fmap<Dep>;
    static_assert (std::is_same_v<Ds, List<Dep<int>, Dep<long>>>);
  }

  namespace
  {
    template<typename...> struct W;
  }

  TEST (UtilTypeList, wrap_rewraps_the_types)
  {
    using Ts = List<int, long>;
    using Ws = typename Ts::wrap<W>;
    static_assert (std::is_same_v<Ws, W<int, long>>);
  }

  TEST (UtilTypeList, wrap_rewraps_the_types_suffixing_given_prefix_types)
  {
    using Ts = List<int, long>;
    static_assert
      ( std::is_same_v< typename Ts::wrap<W, char>
                      , W<char, int, long>
                      >
      );
    static_assert
      ( std::is_same_v< typename Ts::wrap<W, char, bool>
                      , W<char, bool, int, long>
                      >
      );
    static_assert
      ( std::is_same_v< typename Ts::wrap<W, char, bool, int>
                      , W<char, bool, int, int, long>
                      >
      );
    static_assert
      ( std::is_same_v< typename Ts::wrap<W, char, bool, int, long>
                      , W<char, bool, int, long, int, long>
                      >
      );
  }

  namespace
  {
    template<typename, typename...> struct WP1;
  }

  TEST (UtilTypeList, wrap_rewraps_the_types_suffixing_a_single_prefix_type)
  {
    using Ts = List<int, long>;
    using Ws = typename Ts::wrap<WP1, char>;
    static_assert (std::is_same_v<Ws, WP1<char, int, long>>);
  }

  namespace
  {
    template<typename, typename, typename...> struct WP2;
  }

  TEST (UtilTypeList, wrap_rewraps_the_types_suffixing_two_prefix_types)
  {
    using Ts = List<int, long>;
    using Ws = typename Ts::wrap<WP2, char, bool>;
    static_assert (std::is_same_v<Ws, WP2<char, bool, int, long>>);
  }
}

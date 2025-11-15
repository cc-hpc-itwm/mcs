// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/State.hpp>
#include <mcs/util/type/ID.hpp>
#include <string>
#include <variant>

namespace mcs::util::type
{
  // A list of types.
  //
  template<typename... Ts>
    struct List
  {
    // Returns: The number of types in the list.
    //
    // EXAMPLE:
    //   static_assert (List<>::size() == 0);
    //   static_assert (List<int, long>::size() == 2);
    //
    [[nodiscard]] static constexpr auto size() noexcept -> std::size_t;

    // Check if a type is contained in the list.
    //
    // EXAMPLE:
    //   using Ts = List<int, std::string>;
    //   static_assert (Ts::contains<int>());
    //   static_assert (Ts::contains<std::string>());
    //   static_assert (!Ts::contains<long>());
    //
    template<typename T>
      [[nodiscard]] static constexpr auto contains() noexcept -> bool;

    // Returns: A type id for a contained type.
    //
    // Fails to compile if T is not contained in the list.
    //
    // EXAMPLE:
    //   using Ts = List<int, std::string>;
    //   auto const id_i {Ts::id<int>()};
    //   auto const id_s {Ts::id<std::string>()};
    //   ASSERT_NE (id_i, id_s);
    //
    using ID = type::ID<Ts...>;

    template<typename T>
      // \todo requires (List<Ts...>::template contains<T>())
      [[nodiscard]] static constexpr auto id() noexcept -> ID;

    // Invoke Runner::template operator()<type_of_id> (args).
    //
    // EXAMPLE:
    //   using Ts = List<int, A>;
    //   Ts::run
    //     ( Ts::id<int>()
    //     , [&]<typename T> (long v)
    //       {
    //         ASSERT_EQ (typeid (T), typeid (int));
    //         ASSERT_EQ (v, 42L);
    //       }
    //     , 42L
    //     );
    //
    template<typename Runner, typename... Args>
      static constexpr auto run (ID, Runner&&, Args&&...);

    // Parse the dependent type Parse<T>::type and invoke
    // Runner::template operator()<type_of_id> (args). The types to
    // parse are tried in the order of the Ts...: If Ts... = (Head,
    // Tail...), then Parse<Head>::type is tried to parse before and
    // of the Parse<Tail>::type...s are tried to parse.
    //
    // If T is the type with the first successful parse of a value x
    // (of Parse<T>::type), then Runner::template operator()<T> (x,
    // args...) is invoked.
    //
    // EXAMPLE:
    //   using Ts = List<int, A>;
    //   template<typename> struct TypeToParse;
    //   template<> struct TypeToParse<int> { using type = int; }
    //   template<> struct TypeToParse<A> { using type = std::string; }
    //   auto const input {std::string {"Beep"}};
    //   auto state {mcs::util::read::make_state (input)};
    //   Ts::run<TypeToParse>
    //       ( state
    //       , [&]<typename T> (auto const& x, long l)
    //         {
    //           ASSERT_EQ (typeid (T), typeid (A));
    //           ASSERT_EQ (typeid (x), typeid (std::string));
    //         }
    //       , 42L
    //       );
    //
    template< template<typename> typename Parse
            , typename Char
            , typename Runner
            , typename... Args
            >
      static auto parse_id_and_run
        ( util::read::State<Char>&
        , Runner&&
        , Args&&...
        );

    // Convenience helper that creates a parse state and calls
    // parse_and_run (state, ...)
    //
    template< template<typename> typename Parse
            , typename Runner
            , typename... Args
            >
      static auto parse_id_and_run
        ( std::string input
        , Runner&&
        , Args&&...
        );

    // An std::variant of the types in the list.
    //
    // EXAMPLE:
    //   using Ts = List<int, long>;
    //   auto v {Ts::Variant {42});
    //   ASSERT_TRUE (std::holds_alternative<int> (v));
    //
    using Variant = std::variant<Ts...>;

    // Create a list of dependent types.
    //
    // EXAMPLE:
    //   template<typename> struct Dep{};
    //   using Ts = List<int, long>;
    //   using Ds = typename Ts::fmap<Dep>;
    //   static_assert (std::is_same_v<Ds, List<Dep<int>, Dep<long>>>);
    //
    template<template<typename> typename Functor>
      using fmap = List<Functor<Ts>...>;

    // Wrap the types of the list in another TypeContainer.
    //
    // EXAMPLE:
    //   using Ts = List<int, A>;
    //
    //   template<typename...> struct W;
    //   using Ws = Ts::wrap<W>;
    //   static_assert (std::is_same_v<Ws, W<int, A>>);
    //
    //   using Wps = Ts::wrap<W, long, char>;
    //   static_assert (std::is_same_v<Wps, W<long, char, int, A>>);
    //
    //   template<typename, typename...> struct WW;
    //   using WWs = Ts::wrap<WW, long>;
    //   static_assert (std::is_same_v<WWs, WW<long, int, A>>);
    //
    template< template<typename...> typename TypeContainer
            , typename... Prefix
            >
      using wrap = TypeContainer<Prefix..., Ts...>;
  };
}

#include "detail/List.ipp"

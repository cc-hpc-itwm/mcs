// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <mcs/config.hpp>
#include <mcs/util/type/List.hpp>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace mcs::util
{
  // HeterogeneousMap is a thread safe mapping from ID to objects of
  // multiple possible types, meant to be used as base for compile
  // time polymorphic containers. Clients are responsible to remember
  // the type that is assigned with a certain id or else they will
  // face dynamic type errors.
  //
  template<typename ID, typename... Ts>
    struct HeterogeneousMap
  {
    using Values = type::List<Ts...>;

    // Extract read or write access token. There might be multiple
    // concurrent read access tokens but only a single write access
    // token.
    //
    struct ReadAccess
    {
      std::shared_lock<std::shared_mutex> _lock;
    };
    struct WriteAccess
    {
      std::unique_lock<std::shared_mutex> _lock;
    };

    [[nodiscard]] auto read_access() const -> ReadAccess;
    [[nodiscard]] auto write_access() const -> WriteAccess;

    // Create an object of T from the constructor arguments args...
    // Returns: The ID of the new object.
    // Post: visit (..., id, fun) will call fun with the created value.
    //
    // EXAMPLE:
    //   struct A { explicit A (long); };
    //   struct B{};
    //   auto hmap {HeterogenousMap<int, A, B>{}};
    //   auto id_a {hmap.create<A> (hmap.write_access(), 42L)};
    //   auto id_b {hmap.create<B> (hmap.write_access())};
    //
    template<typename T, typename... Args>
      requires (   (std::is_same_v<T, Ts> || ...)
                && std::is_constructible_v<T, Args...>
               )
      [[nodiscard]] auto create (WriteAccess const&, Args&&...) -> ID;

    // Remove the object with the given id.
    // Post: visit (..., id, ...) will throw Error::UnknownID
    //
    // EXAMPLE:
    //   {
    //     auto const write_access {hmap.write_access()};
    //     std::ranges::for_each
    //       ( ids
    //       , [&] (auto id)
    //         {
    //           hmap.remove (write_access, id);
    //         }
    //       );
    //
    auto remove (WriteAccess const&, ID) -> void;

    // Direct access to the elements with a certain id.
    //
    // EXAMPLE:
    //   auto hmap {HeterogenousMap<int, std::string>{}};
    //   auto const id {hmap.create<int> (hmap.write_access(), 42)};
    //   ASSERT_EQ (std::get<int> (hmap.at (id)), 42);
    //
    [[nodiscard]] auto at
      ( ReadAccess const&
      , ID
      ) const -> std::variant<Ts...> const&
      ;
    [[nodiscard]] auto at
      ( WriteAccess const&
      , ID
      ) -> std::variant<Ts...>&
      ;

    // Searches for the element with the given id and calls the
    // continuation with the associated value as parameter.
    // Throws Error::UnknownID if the id has no associated value.
    // Returns: std::invoke (fun, value_associated_with_id, args...)
    //
    // EXAMPLE
    //   auto hmap {HeterogenousMap<int, int, std::string>{}};
    //   auto const id {hmap.create<int> (hmap.write_access(), 42)};
    //   hmap.visit
    //     ( hmap.read_access()
    //     , id
    //     , [] (auto const& x)
    //       {
    //         ASSERT_EQ (typeid (x), typeid (int));
    //         ASSERT_TRUE (EqualTo{}, x, 42);
    //       }
    //     );
    //
    template<typename Fun>
      [[nodiscard]] auto visit
        ( ReadAccess const&
        , ID
        , Fun&&
        ) const;
    template<typename Fun>
      [[nodiscard]] auto visit
        ( WriteAccess const&
        , ID
        , Fun&&
        );

    struct Error
    {
      struct UnknownID : public mcs::Error
      {
        auto id() const -> ID;

        MCS_ERROR_COPY_MOVE_DEFAULT (UnknownID);

      private:
        friend struct HeterogeneousMap;

        explicit UnknownID (ID);

        ID _id;
      };

      struct AccessTokenDoesNotBelongToThis : public mcs::Error
      {
        MCS_ERROR_COPY_MOVE_DEFAULT (AccessTokenDoesNotBelongToThis);

      private:
        friend struct HeterogeneousMap;

        AccessTokenDoesNotBelongToThis();
      };
    };

  private:
    mutable std::shared_mutex _guard;
#if not defined (MCS_CONFIG_GCC_WORKAROUND_BROKEN_DEFAULT_CONSTRUCTOR_LINKAGE)
    ID _next_id{};
#else
    ID _next_id;
#endif
    std::unordered_map<ID, std::variant<Ts...>> _element_by_id;

    template<typename AccessToken>
      auto assert_access_token_belong_to_this
        ( AccessToken const&
        ) const -> void;
  };
}

#include "detail/HeterogeneousMap.ipp"

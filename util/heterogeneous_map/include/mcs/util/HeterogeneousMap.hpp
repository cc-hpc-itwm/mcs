// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <mcs/config.hpp>
#include <mcs/util/Lock.hpp>
#include <mcs/util/heterogeneous_map/Concepts.hpp>
#include <mcs/util/lock/queue/Fast.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace mcs::util
{
  // Map from Key to objects of multiple possible types, meant to be
  // used as base for compile time polymorphic containers. If clients
  // are using the typed interface, then the clients are responsible
  // to use the type that is assigned with a certain key or else they
  // will face dynamic type errors.
  //
  template<heterogeneous_map::is_key Key, typename Types>
    struct UnsynchronizedHeterogeneousMap;

  template<heterogeneous_map::is_key Key, typename... Ts>
    struct UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>
      : public type::List<Ts...>
  {
    using Values = type::List<Ts...>;

    // Create an object of T from the constructor arguments args...
    // Returns: The Key of the new object.
    //
    // EXAMPLE:
    //   struct A { explicit A (long); };
    //   struct B{};
    //   auto ab_map {UnsynchronizedHeterogeneousMap<int, A, B>{}};
    //   auto key_a {ab_map.template create<A> (42L)};
    //   auto key_b {ab_map.template create<B>()};
    //
    template<typename T, typename... Args>
      requires (  (std::is_same_v<T, Ts> || ...)
               && std::is_constructible_v<T, Args...>
               )
      [[nodiscard]] auto create (Args&&...) -> Key;

    // Remove the object with the given key.
    //
    auto remove (Key key) -> void;

    // Direct access to the variant of objects with the given key.
    //
    // EXAMPLE:
    //   std::visit
    //     ( util::overloaded
    //       { [] (A const&) const { fmt::print ("A\n"); }
    //       , [] (B const&) const { fmt::print ("B\n"); }
    //       }
    //     , ab_map.at (key)
    //     );
    //
    [[nodiscard]] auto at (Key) const -> std::variant<Ts...> const&;
    [[nodiscard]] auto at (Key) -> std::variant<Ts...>&;

    // Untyped direct access to an element with a certain key.
    //
    // EXAMPLE
    //   auto hmap {UnsynchronizedHeterogeneousMap<int, int, std::string>{}};
    //   auto const key {hmap.template create<int> (42)};
    //   hmap.visit
    //     ( key
    //     , [] (auto const& x)
    //       {
    //         ASSERT_EQ (typekey (x), typekey (int));
    //         ASSERT_TRUE (EqualTo{}, x, 42);
    //       }
    //     );
    //
    template<typename Fun>
      requires (std::invocable<Fun, Ts const&> && ...)
      auto visit (Key, Fun&&) const;
    template<typename Fun>
      requires (std::invocable<Fun, Ts&> && ...)
      auto visit (Key, Fun&&);

    // Typed direct access to an element with a certain key. If the
    // type does not match, then an exception is thrown.
    //
    // EXAMPLE
    //   auto hmap {UnsynchronizedHeterogeneousMap<int, int, std::string>{}};
    //   auto const key {hmap.template create<int> (42)};
    //   hmap.template invoke<int>
    //     ( key
    //     , [] (int x)
    //       {
    //         ASSERT_EQ (typekey (x), typekey (int));
    //         ASSERT_TRUE (EqualTo{}, x, 42);
    //       }
    //     );
    //
    template<typename T, typename Fun>
      requires (  (std::is_same_v<T, Ts> || ...)
               && std::invocable<Fun, T const&>
               )
      auto invoke (Key, Fun&&) const;

    template<typename T, typename Fun>
      requires (  (std::is_same_v<T, Ts> || ...)
               && std::invocable<Fun, T&>
               )
      auto modify (Key, Fun&&);

    struct Error
    {
      struct UnknownKey : public mcs::Error
      {
        auto key() const -> Key;

        ~UnknownKey() override;
        UnknownKey (UnknownKey const&) = default;
        UnknownKey (UnknownKey&&) noexcept = default;
        auto operator= (UnknownKey const&) -> UnknownKey& = default;
        auto operator= (UnknownKey&&) noexcept  -> UnknownKey& = default;

      private:
        friend struct UnsynchronizedHeterogeneousMap;

        explicit UnknownKey (Key);

        Key _key;
      };
    };

  private:
    Key _next_key{};
    std::unordered_map<Key, std::variant<Ts...>> _element_by_key;
  };

  // Thread safe version of UnsynchronizedHeterogeneousMap.
  //
  // Concurrency management works via construction of objects that
  // offer read access or read&write access. Many read access objects
  // can exists at the same time. Only a single read&write access
  // object can exists at the same time. Read access and read&write
  // access can not exists at the same time.
  //
  template<heterogeneous_map::is_key Key, typename Types>
    struct HeterogeneousMap;

  template<heterogeneous_map::is_key Key, typename... Ts>
    struct HeterogeneousMap<Key, util::type::List<Ts...>>
      : private UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>
  {
    using Base = UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>;

    using Base::Base;
    using typename Base::Values;
    using typename Base::ID;
    using Base::id;
    using typename Base::Error;

    template<lock::is_mode Mode>
      struct Locked : private Lock<Mode, lock::queue::Fast>
    {
      // some inline function definitions to make clang accept

      template<typename Fun>
        requires (std::invocable<Fun, Ts const&> && ...)
        auto visit (Key key, Fun&& fun) const
      {
        return _base->template visit<Fun> (key, std::forward<Fun> (fun));
      }

      template<typename T, typename Fun>
        requires (  (std::is_same_v<T, Ts> || ...)
                 && std::invocable<Fun, T const&>
                 )
        auto invoke (Key key, Fun&& fun) const
      {
        return _base->template invoke<T, Fun> (key, std::forward<Fun> (fun));
      }

    private:
      friend struct HeterogeneousMap;
      UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>> const* _base;
      template<typename... LockArgs>
        Locked
          ( UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>> const*
          , LockArgs&&...
          );
    };
    using ReadAccess = Locked<lock::mode::Shared>;

    struct ReadWriteAccess : public Locked<lock::mode::Unique>
    {
      // some inline function definitions to make clang accept

      using Locked<lock::mode::Unique>::visit;
      using Locked<lock::mode::Unique>::invoke;

      template<typename T, typename... Args>
        requires (  (std::is_same_v<T, Ts> || ...)
                 && std::is_constructible_v<T, Args...>
                 )
        [[nodiscard]] auto create (Args&&... args) const -> Key
      {
        return _base->template create<T> (std::forward<Args> (args)...);
      }

      auto remove (Key key) const -> void;

      template<typename Fun>
        requires (std::invocable<Fun, Ts&> && ...)
        auto visit (Key key, Fun&& fun) const
      {
        return _base->template visit<Fun> (key, std::forward<Fun> (fun));
      }

      template<typename T, typename Fun>
        requires (  (std::is_same_v<T, Ts> || ...)
                 && std::invocable<Fun, T&>
                 )
        auto modify (Key key, Fun&& fun) const
      {
        return _base->template modify<T, Fun> (key, std::forward<Fun> (fun));
      }

    private:
      friend struct HeterogeneousMap;
      UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>* _base;
      template<typename... LockArgs>
        ReadWriteAccess
          ( UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>*
          , LockArgs&&...
          );
    };

    // Only a single ReadWriteAccess can exist at the same
    // time. ReadAccess and ReadWriteAccess can not exist at the same
    // time. To acquire a ReadWriteAccess has priority over acquiring
    // ReadAccess: ReadWriteAccess access will be granted after all
    // current ReadAccesses have been released and before ReadAccesses
    // accesses are granted that are requested after the request for
    // ReadWriteAccess access has been granted. Sequences of
    // ReadWriteAccess delay ReadAccess indefinitely long.
    //
    [[nodiscard]] auto read_access() const -> ReadAccess;
    [[nodiscard]] auto read_write_access() -> ReadWriteAccess;

  private:
    lock::SharedMutex<lock::queue::Fast> _guard;
  };
}

#include "detail/HeterogeneousMap.ipp"

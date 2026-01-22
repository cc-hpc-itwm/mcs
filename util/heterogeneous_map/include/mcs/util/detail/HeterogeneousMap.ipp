// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <iterator>
#include <mcs/nonstd/scope.hpp>
#include <memory>
#include <utility>

namespace mcs::util
{
  template<heterogeneous_map::is_key Key, typename... Ts>
    template<typename T, typename... Args>
      requires (  (std::is_same_v<T, Ts> || ...)
               && std::is_constructible_v<T, Args...>
               )
    auto UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>::create
      ( Args&&... args
      ) -> Key
  {
    auto const increment_key
      { nonstd::make_scope_exit
        ( [&]() noexcept
          {
            std::ignore = ++_next_key;
          }
        )
      };

    return _element_by_key.emplace
      ( std::piecewise_construct
      , std::forward_as_tuple (_next_key)
      , std::forward_as_tuple
        ( std::in_place_type_t<T>{}
        , std::forward<Args> (args)...
        )
      ).first->first
      ;
  }

  template<heterogeneous_map::is_key Key, typename... Ts>
    auto UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>
      ::remove (Key key) -> void
  {
    _element_by_key.erase (key);
  }

  template<heterogeneous_map::is_key Key, typename... Ts>
    auto UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>::at
      ( Key key
      ) const -> std::variant<Ts...> const&
  {
    auto const element {_element_by_key.find (key)};

    if (element == std::cend (_element_by_key))
    {
      throw typename Error::UnknownKey {key};
    }

    return element->second;
  }

  template<heterogeneous_map::is_key Key, typename... Ts>
    auto UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>::at
      ( Key key
      ) -> std::variant<Ts...>&
  {
    auto const element {_element_by_key.find (key)};

    if (element == std::end (_element_by_key))
    {
      throw typename Error::UnknownKey {key};
    }

    return element->second;
  }

  template<heterogeneous_map::is_key Key, typename... Ts>
    template<typename Fun>
    requires (std::invocable<Fun, Ts const&> && ...)
      auto UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>::visit
        ( Key key
        , Fun&& fun
        ) const
  {
    return std::visit (std::forward<Fun> (fun), at (key));
  }

  template<heterogeneous_map::is_key Key, typename... Ts>
    template<typename Fun>
    requires (std::invocable<Fun, Ts&> && ...)
      auto UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>::visit
        ( Key key
        , Fun&& fun
        )
  {
    return std::visit (std::forward<Fun> (fun), at (key));
  }

  template<heterogeneous_map::is_key Key, typename... Ts>
    template<typename T, typename Fun>
    requires (  (std::is_same_v<T, Ts> || ...)
             && std::invocable<Fun, T const&>
             )
      auto UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>::invoke
        ( Key key
        , Fun&& fun
        ) const
  {
    return std::invoke (std::forward<Fun> (fun), std::get<T> (at (key)));
  }

  template<heterogeneous_map::is_key Key, typename... Ts>
    template<typename T, typename Fun>
    requires (  (std::is_same_v<T, Ts> || ...)
             && std::invocable<Fun, T&>
             )
      auto UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>::modify
        ( Key key
        , Fun&& fun
        )
  {
    return std::invoke (std::forward<Fun> (fun), std::get<T> (at (key)));
  }
}

namespace mcs::util
{
  template<heterogeneous_map::is_key Key, typename... Ts>
    UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>::Error::UnknownKey::UnknownKey
      ( Key key
      )
        : mcs::Error {fmt::format ("Unknown key '{}'", key)}
        , _key {key}
  {}
  template<heterogeneous_map::is_key Key, typename... Ts>
    auto UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>::Error::UnknownKey::key
      (
      ) const -> Key
  {
    return _key;
  }
  template<heterogeneous_map::is_key Key, typename... Ts>
    UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>::Error::UnknownKey::~UnknownKey
      (
      ) = default
    ;
}

namespace mcs::util
{
  template<heterogeneous_map::is_key Key, typename... Ts>
    template<lock::is_mode Mode>
      template<typename... LockArgs>
        HeterogeneousMap<Key, util::type::List<Ts...>>::Locked<Mode>::Locked
          ( UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>> const* base
          , LockArgs&&... lock_args
          )
            : Lock<Mode, lock::queue::Fast>
              { std::forward<LockArgs> (lock_args)...
              }
            , _base {base}
  {}
}

namespace mcs::util
{
  template<heterogeneous_map::is_key Key, typename... Ts>
    template<typename... LockArgs>
      HeterogeneousMap<Key, util::type::List<Ts...>>::ReadWriteAccess::ReadWriteAccess
        ( UnsynchronizedHeterogeneousMap<Key, util::type::List<Ts...>>* base
        , LockArgs&&... lock_args
        )
          : Locked<lock::mode::Unique> {base, std::forward<LockArgs> (lock_args)...}
          , _base {base}
  {}

  template<heterogeneous_map::is_key Key, typename... Ts>
    auto HeterogeneousMap<Key, util::type::List<Ts...>>::ReadWriteAccess::remove
      ( Key key
      ) const -> void
  {
    return _base->remove (key);
  }
}

namespace mcs::util
{
  template<heterogeneous_map::is_key Key, typename... Ts>
    auto HeterogeneousMap<Key, util::type::List<Ts...>>::read_access() const -> ReadAccess
  {
    return ReadAccess {this, std::addressof (_guard)};
  }

  template<heterogeneous_map::is_key Key, typename... Ts>
    auto HeterogeneousMap<Key, util::type::List<Ts...>>::read_write_access() -> ReadWriteAccess
  {
    return ReadWriteAccess {this, std::addressof (_guard)};
  }
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <iterator>
#include <mcs/nonstd/scope.hpp>
#include <memory>
#include <utility>

namespace mcs::util
{
  template<typename ID, typename... Ts>
    auto HeterogeneousMap<ID, Ts...>::read_access() const -> ReadAccess
  {
    return ReadAccess {std::shared_lock {_guard}};
  }
  template<typename ID, typename... Ts>
    auto HeterogeneousMap<ID, Ts...>::write_access() const -> WriteAccess
  {
    return WriteAccess {std::unique_lock {_guard}};
  }

  template<typename ID, typename... Ts>
    template<typename AccessToken>
      auto HeterogeneousMap<ID, Ts...>::assert_access_token_belong_to_this
        ( AccessToken const& access_token
        ) const -> void
  {
    if (! (access_token._lock.mutex() == std::addressof (_guard)))
    {
      throw typename Error::AccessTokenDoesNotBelongToThis{};
    }
  }

  template<typename ID, typename... Ts>
    template<typename T, typename... Args>
      requires (   (std::is_same_v<T, Ts> || ...)
                && std::is_constructible_v<T, Args...>
               )
      auto HeterogeneousMap<ID, Ts...>::create
        ( WriteAccess const& write_access
        , Args&&... args
        ) -> ID
  {
    assert_access_token_belong_to_this (write_access);

    auto const increment_id
      { nonstd::make_scope_exit
        ( [&]() noexcept
          {
            ++_next_id;
          }
        )
      };

    return _element_by_id.emplace
      ( std::piecewise_construct
      , std::forward_as_tuple (_next_id)
      , std::forward_as_tuple
        ( std::in_place_type_t<T>{}
        , std::forward<Args> (args)...
        )
      ).first->first
      ;
  }

  template<typename ID, typename... Ts>
    auto HeterogeneousMap<ID, Ts...>::remove
      ( WriteAccess const& write_access
      , ID id
      ) -> void
  {
    assert_access_token_belong_to_this (write_access);

    _element_by_id.erase (id);
  }

  template<typename ID, typename... Ts>
    HeterogeneousMap<ID, Ts...>::Error::UnknownID::UnknownID (ID id)
      : mcs::Error {fmt::format ("Unknown id '{}'", id)}
      , _id {id}
  {}
  template<typename ID, typename... Ts>
    auto HeterogeneousMap<ID, Ts...>::Error::UnknownID::id() const -> ID
  {
    return _id;
  }
  template<typename ID, typename... Ts>
    HeterogeneousMap<ID, Ts...>::Error::UnknownID::~UnknownID() = default;

  template<typename ID, typename... Ts>
    HeterogeneousMap<ID, Ts...>::Error::AccessTokenDoesNotBelongToThis::AccessTokenDoesNotBelongToThis()
      : mcs::Error {"Access token does not belong to this."}
  {}
  template<typename ID, typename... Ts>
    HeterogeneousMap<ID, Ts...>::Error::AccessTokenDoesNotBelongToThis::~AccessTokenDoesNotBelongToThis() = default;

#define MCS_UTIL_VARIANT_ID_MAP_AT_IMPL()                       \
  auto const element {_element_by_id.find (id)};                \
                                                                \
  if (element == std::end (_element_by_id))                     \
  {                                                             \
    throw typename Error::UnknownID {id};                       \
  }                                                             \
                                                                \
  return element->second

  template<typename ID, typename... Ts>
    auto HeterogeneousMap<ID, Ts...>::at
      ( ReadAccess const& read_access
      , ID id
      ) const -> std::variant<Ts...> const&
  {
    assert_access_token_belong_to_this (read_access);

    MCS_UTIL_VARIANT_ID_MAP_AT_IMPL();
  }

  template<typename ID, typename... Ts>
    auto HeterogeneousMap<ID, Ts...>::at
      ( WriteAccess const& write_access
      , ID id
      ) -> std::variant<Ts...>&
  {
    assert_access_token_belong_to_this (write_access);

    MCS_UTIL_VARIANT_ID_MAP_AT_IMPL();
  }

  template<typename ID, typename... Ts>
    template<typename Fun>
      auto HeterogeneousMap<ID, Ts...>::visit
        ( ReadAccess const& read_access
        , ID id
        , Fun&& fun
        ) const
  {
    return std::visit (std::forward<Fun> (fun), at (read_access, id));
  }
  template<typename ID, typename... Ts>
    template<typename Fun>
      auto HeterogeneousMap<ID, Ts...>::visit
        ( WriteAccess const& write_access
        , ID id
        , Fun&& fun
        )
  {
    return std::visit (std::forward<Fun> (fun), at (write_access, id));
  }

#undef MCS_UTIL_VARIANT_ID_MAP_VISIT_IMPL
}

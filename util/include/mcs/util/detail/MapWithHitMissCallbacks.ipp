// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <tuple>
#include <utility>

namespace mcs::util
{
  template<typename Key, typename Value, typename Map>
    template<typename Hit, typename Miss>
    requires (   is_hit_callback<Hit, Key, Map>
             and is_miss_callback<Miss, Key, Map>
             )
    auto MapWithHitMissCallbacks<Key, Value, Map>::find
        ( Key key
        , Hit&& hit
        , Miss&& miss
        ) -> std::optional<typename Map::iterator>
  {
    auto element {_map.find (key)};

    if (element != std::end (_map))
    {
      std::invoke
        (std::forward<Hit> (hit), key, static_cast<Map const&> (_map));

      return element;
    }

    std::invoke (std::forward<Miss> (miss), key, _map);

    return {};
  }

  template<typename Key, typename Value, typename Map>
    template<typename Hit, typename Miss, typename... Args>
    requires (   is_hit_callback<Hit, Key, Map>
             and is_miss_callback<Miss, Key, Map>
             and std::is_constructible_v<Value, Args...>
             )
    auto MapWithHitMissCallbacks<Key, Value, Map>::at_or_construct
        ( Key key
        , Hit&& hit
        , Miss&& miss
        , Args&&... args
        ) -> Value&
    {
      if ( auto element
             {find (key, std::forward<Hit> (hit), std::forward<Miss> (miss))}
         )
      {
        return (*element)->second;
      }

      return _map.try_emplace
        ( key
        , std::forward<Args> (args)...
        ).first->second
      ;
    }

  template<typename Key, typename Value, typename Map>
    template<typename Hit, typename Miss, typename Create, typename... Args>
    requires (   is_hit_callback<Hit, Key, Map>
             and is_miss_callback<Miss, Key, Map>
             and std::is_invocable_r_v<Value, Create, Args...>
             )
    auto MapWithHitMissCallbacks<Key, Value, Map>::at_or_create
        ( Key key
        , Hit&& hit
        , Miss&& miss
        , Create&& create
        , Args&&... args
        ) -> Value&
    {
      if ( auto element
             {find (key, std::forward<Hit> (hit), std::forward<Miss> (miss))}
         )
      {
        return (*element)->second;
      }

      return _map.emplace
        ( std::move (key)
        , std::invoke ( std::forward<Create> (create)
                      , std::forward<Args> (args)...
                      )
        ).first->second
      ;
    }
}

namespace mcs::util
{
  template<typename Key, typename Value, typename Map>
    template<typename Hit, typename Miss, typename... Args>
    requires (   is_hit_callback<Hit, Key, Map>
             and is_miss_callback<Miss, Key, Map>
             and std::is_constructible_v<Value, Args...>
             )
    auto ConcurrentMapWithHitMissCallbacks<Key, Value, Map>::at_or_construct
        ( Key key
        , Hit&& hit
        , Miss&& miss
        , Args&&... args
        ) -> Value
    {
      auto const lock {std::lock_guard {_guard}};

      return _map.at_or_construct
        ( std::move (key)
        , std::forward<Hit> (hit)
        , std::forward<Miss> (miss)
        , std::forward<Args> (args)...
        );
    }

  template<typename Key, typename Value, typename Map>
    template<typename Hit, typename Miss, typename Create, typename... Args>
    requires (   is_hit_callback<Hit, Key, Map>
             and is_miss_callback<Miss, Key, Map>
             and std::is_invocable_r_v<Value, Create, Args...>
             )
    auto ConcurrentMapWithHitMissCallbacks<Key, Value, Map>::at_or_create
        ( Key key
        , Hit&& hit
        , Miss&& miss
        , Create&& create
        , Args&&... args
        ) -> Value
    {
      auto const lock {std::lock_guard {_guard}};

      return _map.at_or_create
        ( std::move (key)
        , std::forward<Hit> (hit)
        , std::forward<Miss> (miss)
        , std::forward<Create> (create)
        , std::forward<Args> (args)...
        );
    }
}

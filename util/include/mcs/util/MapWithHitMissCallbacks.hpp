// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <functional>
#include <iterator>
#include <mutex>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace mcs::util
{
  // \todo access_policy, make thread safe, test access with multiple threads
  template<typename Map, typename Key, typename Value>
    concept is_associative_map = requires (Map map, Key key, Value&& value)
    {
      {map.find (key) != std::end (map)} -> std::convertible_to<bool>;
      {map.find (key)->second} -> std::convertible_to<Value&>;
      {map.emplace (key, value).first->second} -> std::convertible_to<Value&>;
    };
    // also: find does not invalidate references
    // also: emplace does not invalidate references

  template<typename Hit, typename Key, typename Map>
    concept is_hit_callback = requires ( Hit&& hit
                                       , Key const& key
                                       , Map const& map
                                       )
    { { std::invoke (std::forward<Hit> (hit), key, map)
      } -> std::convertible_to<void>
      ;
    };
    // also: hit does not invalidate references

  template<typename Miss, typename Key, typename Map>
    concept is_miss_callback = requires ( Miss&& miss
                                        , Key const& key
                                        , Map& map
                                        )
    { { std::invoke (std::forward<Miss> (miss), key, map)
      } -> std::convertible_to<void>
      ;
    };
    // also: miss does not invalidate references

  // A cache with callbacks for hit and miss events.
  //
  template< typename Key
          , typename Value
          , typename Map = std::unordered_map<Key, Value>
          >
    struct MapWithHitMissCallbacks
  {
    static_assert (is_associative_map<Map, Key, Value>);

    // Returns: A reference to the value associated with key.
    //
    // If key has a value associated already, then `hit (key, map)` is
    // called and a reference to the associated value is returned. If
    // the key does not exist, then `miss (key, map)` is called and
    // key is associated with a value constructed from the provided
    // arguments. A reference to the new created value is returned.
    //
    template<typename Hit, typename Miss, typename... Args>
      requires (   is_hit_callback<Hit, Key, Map>
               and is_miss_callback<Miss, Key, Map>
               and std::is_constructible_v<Value, Args...>
               )
      auto at_or_construct (Key, Hit&&, Miss&&, Args&&...) -> Value&;

    // Returns: A reference to the value associated with key.
    //
    // If key has a value associated already, then `hit (key, map)` is
    // called and a reference to the associated value is returned. If
    // the key does not exist, then `miss (key, map)` is called and
    // key is associated with a value constructed from a call to
    // `create (args...)`. A reference to the new created value is
    // returned.
    //
    template<typename Hit, typename Miss, typename Create, typename... Args>
      requires (   is_hit_callback<Hit, Key, Map>
               and is_miss_callback<Miss, Key, Map>
               and std::is_invocable_r_v<Value, Create, Args...>
               )
      auto at_or_create (Key, Hit&&, Miss&&, Create&&, Args&&...) -> Value&;

  private:
    Map _map;

    template<typename Hit, typename Miss>
      requires (   is_hit_callback<Hit, Key, Map>
               and is_miss_callback<Miss, Key, Map>
               )
      auto find
        ( Key
        , Hit&&
        , Miss&&
        ) -> std::optional<typename Map::iterator>
      ;
  };

  // A thread-safe cache with callbacks for hit and miss events. The
  // difference in the API is that copies of the current values are
  // returned rather than references that might be invalidated by
  // other threads.
  //
  template< typename Key
          , typename Value
          , typename Map = std::unordered_map<Key, Value>
          >
    struct ConcurrentMapWithHitMissCallbacks
  {
    static_assert (is_associative_map<Map, Key, Value>);

    // Returns: A copy of the value associated with key.
    //
    // If key has a value associated already, then `hit (key, map)` is
    // called and a reference to the associated value is returned. If
    // the key does not exist, then `miss (key, map)` is called and
    // key is associated with a value constructed from the provided
    // arguments. A copy of the new created value is returned.
    //
    template<typename Hit, typename Miss, typename... Args>
      requires (   is_hit_callback<Hit, Key, Map>
               and is_miss_callback<Miss, Key, Map>
               and std::is_constructible_v<Value, Args...>
               )
      auto at_or_construct (Key, Hit&&, Miss&&, Args&&...) -> Value;

    // Returns: A copy of the value associated with key.
    //
    // If key has a value associated already, then `hit (key, map)` is
    // called and a reference to the associated value is returned. If
    // the key does not exist, then `miss (key, map)` is called and
    // key is associated with a value constructed from a call to
    // `create (args...)`. A copy of the new created value is
    // returned.
    //
    template<typename Hit, typename Miss, typename Create, typename... Args>
      requires (   is_hit_callback<Hit, Key, Map>
               and is_miss_callback<Miss, Key, Map>
               and std::is_invocable_r_v<Value, Create, Args...>
               )
      auto at_or_create (Key, Hit&&, Miss&&, Create&&, Args&&...) -> Value;

  private:
    std::mutex _guard;
    MapWithHitMissCallbacks<Key, Value, Map> _map;
  };
}

#include "detail/MapWithHitMissCallbacks.ipp"

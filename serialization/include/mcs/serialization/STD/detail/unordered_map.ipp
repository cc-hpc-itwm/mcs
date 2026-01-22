// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::serialization
{
  template< is_serializable Key
          , is_serializable T
          , is_serializable Hash
          , is_serializable KeyEqual
          , is_serializable Allocator
          >
    auto Implementation<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>>::output
      ( OArchive& oa
      , std::unordered_map<Key, T, Hash, KeyEqual, Allocator> const& m
      ) -> OArchive&
  {
    oa.tag<detail::tag::STD::UnorderedMap> (m.size(), m.bucket_count());

    save (oa, m.hash_function());
    save (oa, m.key_eq());
    save (oa, m.get_allocator());

    std::ranges::for_each
      ( m
      , [&] (auto const& kv)
        {
          save (oa, kv.first);
          save (oa, kv.second);
        }
      );

    return oa;
  }

  template< is_serializable Key
          , is_serializable T
          , is_serializable Hash
          , is_serializable KeyEqual
          , is_serializable Allocator
          >
    auto Implementation<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>>::input
      ( IArchive& ia
      ) -> std::unordered_map<Key, T, Hash, KeyEqual, Allocator>
  {
    auto [size, bucket_count] {ia.tag<detail::tag::STD::UnorderedMap>()};

    auto hash {load<Hash> (ia)};
    auto key_eq {load<KeyEqual> (ia)};
    auto allocator {load<Allocator> (ia)};
    auto m { std::unordered_map<Key, T, Hash, KeyEqual, Allocator>
               { bucket_count
               , std::move (hash)
               , std::move (key_eq)
               , std::move (allocator)
               }
           };

    while (size --> 0)
    {
      auto key {load<Key> (ia)};
      auto value {load<T> (ia)};
      m.emplace (std::move (key), std::move (value));
    }

    return m;
  }
}

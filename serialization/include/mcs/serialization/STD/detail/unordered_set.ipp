// Copyright (C) 2026 Fraunhofer ITWM
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
          , is_serializable Hash
          , is_serializable KeyEqual
          , is_serializable Allocator
          >
    auto Implementation<std::unordered_set<Key, Hash, KeyEqual, Allocator>>::output
      ( OArchive& oa
      , std::unordered_set<Key, Hash, KeyEqual, Allocator> const& s
      ) -> OArchive&
  {
    oa.tag<detail::tag::STD::UnorderedSet> (s.size(), s.bucket_count());

    save (oa, s.hash_function());
    save (oa, s.key_eq());
    save (oa, s.get_allocator());

    std::ranges::for_each
      ( s
      , [&] (auto const& x)
        {
          save (oa, x);
        }
      );

    return oa;
  }

  template< is_serializable Key
          , is_serializable Hash
          , is_serializable KeyEqual
          , is_serializable Allocator
          >
    auto Implementation<std::unordered_set<Key, Hash, KeyEqual, Allocator>>::input
      ( IArchive& ia
      ) -> std::unordered_set<Key, Hash, KeyEqual, Allocator>
  {
    auto [size, bucket_count] {ia.tag<detail::tag::STD::UnorderedSet>()};

    auto hash {load<Hash> (ia)};
    auto key_eq {load<KeyEqual> (ia)};
    auto allocator {load<Allocator> (ia)};
    auto s { std::unordered_set<Key, Hash, KeyEqual, Allocator>
               { bucket_count
               , std::move (hash)
               , std::move (key_eq)
               , std::move (allocator)
               }
           };

    while (size --> 0)
    {
      auto x {load<Key> (ia)};
      s.emplace (std::move (x));
    }

    return s;
  }
}

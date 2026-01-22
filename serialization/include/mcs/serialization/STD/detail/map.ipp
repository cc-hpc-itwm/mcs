// Copyright (C) 2022-2025 Fraunhofer ITWM
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
          , is_serializable Compare
          , is_serializable Allocator
          >
    auto Implementation<std::map<Key, T, Compare, Allocator>>::output
      ( OArchive& oa
      , std::map<Key, T, Compare, Allocator> const& m
      ) -> OArchive&
  {
    oa.tag<detail::tag::STD::Map> (m.size());

    save (oa, m.key_comp());
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
          , is_serializable Compare
          , is_serializable Allocator
          >
    auto Implementation<std::map<Key, T, Compare, Allocator>>::input
      ( IArchive& ia
      ) -> std::map<Key, T, Compare, Allocator>
  {
    auto size {ia.tag<detail::tag::STD::Map>().size};

    auto compare {load<Compare> (ia)};
    auto allocator {load<Allocator> (ia)};
    auto m { std::map<Key, T, Compare, Allocator>
               {std::move (compare), std::move (allocator)}
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

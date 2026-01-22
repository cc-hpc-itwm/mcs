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
          , is_serializable Compare
          , is_serializable Allocator
          >
    auto Implementation<std::set<Key, Compare, Allocator>>::output
      ( OArchive& oa
      , std::set<Key, Compare, Allocator> const& xs
      ) -> OArchive&
  {
    oa.tag<detail::tag::STD::Set> (xs.size());

    save (oa, xs.key_comp());
    save (oa, xs.get_allocator());

    std::ranges::for_each
      ( xs
      , [&] (auto const& x)
        {
          save (oa, x);
        }
      );

    return oa;
  }

  template< is_serializable Key
          , is_serializable Compare
          , is_serializable Allocator
          >
    auto Implementation<std::set<Key, Compare, Allocator>>::input
      ( IArchive& ia
      ) -> std::set<Key, Compare, Allocator>
  {
    auto size {ia.tag<detail::tag::STD::Set>().size};

    auto compare {load<Compare> (ia)};
    auto allocator {load<Allocator> (ia)};
    auto xs { std::set<Key, Compare, Allocator>
                {std::move (compare), std::move (allocator)}
            };

    while (size --> 0)
    {
      xs.emplace (load<Key> (ia));
    }

    return xs;
  }
}

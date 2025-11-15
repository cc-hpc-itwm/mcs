// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <mcs/serialization/define.hpp>
#include <utility>

namespace mcs::serialization
{
  template<is_serializable T, is_serializable Allocator>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , xs
      , std::list<T, Allocator>
      )
  {
    oa.tag<detail::tag::STD::List> (xs.size());

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

  template<is_serializable T, is_serializable Allocator>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , std::list<T, Allocator>
      )
  {
    auto size {ia.tag<detail::tag::STD::List>().size};

    auto allocator {load<Allocator> (ia)};
    auto xs {std::list<T, Allocator> {std::move (allocator)}};

    while (size --> 0)
    {
      xs.emplace_back (load<T> (ia));
    }

    return xs;
  }
}

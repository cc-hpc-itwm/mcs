// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <mcs/serialization/define.hpp>
#include <type_traits>

namespace mcs::serialization
{
  template<is_serializable T, is_serializable Allocator>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , xs
      , std::vector<T, Allocator>
      )
  {
    oa.tag<detail::tag::STD::Vector> (xs.size());

    save (oa, xs.get_allocator());

    if constexpr (std::is_trivially_copyable_v<T>)
    {
      oa.append (std::span {xs});
    }
    else
    {
      std::ranges::for_each
        ( xs
        , [&] (auto const& x)
          {
            save (oa, x);
          }
        );
    }

    return oa;
  }

  template<is_serializable T, is_serializable Allocator>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , std::vector<T, Allocator>
      )
  {
    auto size {ia.tag<detail::tag::STD::Vector>().size};

    auto xs {std::vector<T, Allocator> {load<Allocator> (ia)}};

    if constexpr (std::is_trivially_copyable_v<T>)
    {
      xs.resize (size);

      ia.extract (xs.data(), size * sizeof (T));
    }
    else
    {
      xs.reserve (size);

      while (size --> 0)
      {
        xs.emplace_back (load<T> (ia));
      }
    }

    return xs;
  }
}

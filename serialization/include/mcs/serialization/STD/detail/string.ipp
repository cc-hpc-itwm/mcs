// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::serialization
{
  template< class CharT
          , class Traits
          , is_serializable Allocator
          >
    auto Implementation<std::basic_string<CharT, Traits, Allocator>>::output
      ( OArchive& oa
      , std::basic_string<CharT, Traits, Allocator> const& s
      ) -> OArchive&
  {
    oa.tag<detail::tag::STD::String> (s.size());

    save (oa, s.get_allocator());

    oa.append (std::span {s});

    return oa;
  }

  template< class CharT
          , class Traits
          , is_serializable Allocator
          >
    auto Implementation<std::basic_string<CharT, Traits, Allocator>>::input
      ( IArchive& ia
      ) -> std::basic_string<CharT, Traits, Allocator>
  {
    auto size {ia.tag<detail::tag::STD::String>().size};

    auto allocator {load<Allocator> (ia)};
    auto s { std::basic_string<CharT, Traits, Allocator>
               {std::move (allocator)}
           };
    s.resize (size);

    ia.extract (s.data(), size * sizeof (CharT));

    return s;
  }
}

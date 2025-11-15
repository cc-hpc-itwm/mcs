// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/define.hpp>
#include <utility>

namespace mcs::serialization
{
  template< class CharT
          , class Traits
          , is_serializable Allocator
          >
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , s
      , std::basic_string<CharT, Traits, Allocator>
      )
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
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , std::basic_string<CharT, Traits, Allocator>
      )
  {
    auto size {ia.tag<detail::tag::STD::String>().size};

    auto allocator {load<Allocator> (ia)};
    auto s { std::basic_string<CharT, Traits, Allocator>
               {std::move (allocator)}
           };
    s.resize (size);

    ia.extract (s.data(), size);

    return s;
  }
}

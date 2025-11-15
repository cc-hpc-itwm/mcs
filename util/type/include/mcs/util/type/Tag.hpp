// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util::type
{
  // A container for a type.
  //
  // EXAMPLE:
  //   struct A { A (int); };
  //   using SupportedType = std::variant<Tag<int>, Tag<A>>;
  //   struct C
  //   {
  //     C (SupportedType supported_type, int v)
  //       : _v { std::visit
  //              ( [&] (auto x) -> decltype (_v)
  //                {
  //                  return typename decltype (t)::type {v};
  //                }
  //              )
  //            }
  //     {}
  //     std::variant<int, A> _v;
  //   };
  //   auto const i {C {Tag<int>{}, 42}};
  //   ASSERT_TRUE (std::holds_alternative<int> (i._v));
  //   auto const a {C {Tag<A>{}, 42}};
  //   ASSERT_TRUE (std::holds_alternative<A> (a._v));
  //
  template<typename T>
    struct Tag
  {
    using type = T;
  };
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/util/type/Tag.hpp>

namespace mcs::util::type
{
  TEST (UtilTypeTag, decoumentation_example_works)
  {
    struct A
    {
      A (int i) : _i {i}
      {}

      int _i;
    };
    using SupportedType = std::variant<Tag<int>, Tag<A>>;
    struct C
    {
      C (SupportedType supported_type, int v)
        : _v { std::visit
               ( [&] (auto x) noexcept -> decltype (_v)
                 {
                   return typename decltype (x)::type {v};
                 }
               , supported_type
               )
             }
      {}
      std::variant<int, A> _v;
    };
    auto const i {C {Tag<int>{}, 42}};
    ASSERT_TRUE (std::holds_alternative<int> (i._v));
    auto const a {C {Tag<A>{}, 42}};
    ASSERT_TRUE (std::holds_alternative<A> (a._v));
  }
}

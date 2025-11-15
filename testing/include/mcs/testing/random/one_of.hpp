// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <ranges>

namespace mcs::testing::random
{
  // returns one of the elements from the range
  //
  // Supports references, const-references and values.
  //
  // EXAMPLES:
  //
  //     // temporary range
  //     auto x {random::one_of (std::vector<int> {0, 1, 2})};
  //
  //     // const range
  //     auto const ys {std::vector<int> {4, 5, 6}};
  //     auto y {random::one_of (ys)};
  //
  //     // range of non-moveable objects
  //     auto zs {std::list<NotMoveable> {...}};
  //     auto& z {random::one_of (zs)};
  //
  template<typename Range>
    auto one_of ( Range const& range
                ) -> decltype (*std::ranges::begin (range)) const&
    ;
  template<typename Range>
    auto one_of ( Range& range
                ) -> decltype (*std::ranges::begin (range))&
    ;
}

#include "detail/one_of.ipp"

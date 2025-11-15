// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include "detail/fwd_capture.ipp"

namespace mcs::util
{
  // https://vittorioromeo.info/index/blog/capturing_perfectly_forwarded_objects_in_lambdas.html
  //
  // Helper to perfect forward capture in lambdas.
  //
  // EXAMPLE:
  //
  //     struct S
  //     {
  //       S (int x) : _x {x} {}
  //       int _x;
  //     };
  //
  //     auto f (auto&& s)
  //     {
  //       return [s = mcs::util::fwd_capture (std::forward<decltype (s)> (s))]
  //         ( int k
  //         ) mutable noexcept
  //       {
  //         return s.get()._x += k;
  //       };
  //     }
  //
  //     // forward mutable reference
  //     auto s {S {42}};
  //     ASSERT_EQ (std::invoke (f (s), 21), 42 + 21);
  //     ASSERT_EQ (s._x, 42 + 21);
  //
  //     // forward rvalue
  //     ASSERT_EQ (std::invoke (f (S {42}, 21), 42 + 21);
  //
  template<typename T>
    auto fwd_capture (T&&);
}

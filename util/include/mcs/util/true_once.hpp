// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util
{
  // Behaves like a boolean that is true iff observed for the first
  // time.
  //
  // EXAMPLE
  //     auto first {TrueOnce{}};
  //     for (auto x : xs)
  //     {
  //       if (!first)
  //       {
  //         os << ", ";
  //       }
  //       os << x;
  //     }
  //
  struct TrueOnce
  {
    [[nodiscard]] constexpr operator bool() const noexcept;

  private:
    mutable bool _value {true};
  };
}

#include "detail/true_once.ipp"

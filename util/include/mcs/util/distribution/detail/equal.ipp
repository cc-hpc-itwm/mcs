// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/divru.hpp>

namespace mcs::util::distribution::equal
{
  constexpr auto begin (auto M, auto P, auto slot)
  {
    return util::divru (slot * M, P);
  }

  constexpr auto end (auto M, auto P, auto slot)
  {
    return begin (M, P, slot + 1);
  }

  constexpr auto size (auto M, auto P, auto slot)
  {
    return end (M, P, slot) - begin (M, P, slot);
  }
}

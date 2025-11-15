// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/State.hpp>

namespace mcs::util::read
{
  // Returns state.head() if it is equal to any of the allowed values.
  //
  template<typename Allowed, typename Char>
    [[nodiscard]] constexpr auto one_of (State<Char>&, Allowed) -> Char;
}

#include "detail/one_of.ipp"

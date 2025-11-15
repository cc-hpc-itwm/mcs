// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/State.hpp>

namespace mcs::util::read
{
  // Requires and skips the given prefix. Skips whitespace first.
  //
  template<typename Prefix, typename Char>
    constexpr auto prefix (State<Char>&, Prefix) -> State<Char>&;

  // Consumes the prefix if it is present. Return true iff the prefix
  // was present.
  //
  template<typename Prefix, typename Char>
    constexpr auto maybe_prefix (State<Char>&, Prefix) -> bool;
}

#include "detail/prefix.ipp"

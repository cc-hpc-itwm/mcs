// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/State.hpp>

namespace mcs::util::read
{
  // Skips all leading spaces as determined by isspace.
  //
  template<typename Char>
    constexpr auto skip_whitespace (State<Char>&) -> void;
}

#include "detail/skip_whitespace.ipp"

// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/State.hpp>

namespace mcs::util::read
{
  template<typename T, typename Char>
    [[nodiscard]] constexpr auto parse (State<Char>&) -> T;
}

#include "detail/parse.ipp"

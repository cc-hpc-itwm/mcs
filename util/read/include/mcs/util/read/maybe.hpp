// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/State.hpp>
#include <optional>

namespace mcs::util::read
{
  // Tries to parse<T> (state) and to return that value. If parse<T>
  // (state) throws an exception, then the state is reset and nothing
  // is returned.
  //
  template<typename T, typename Char>
    constexpr auto maybe (State<Char>&) -> std::optional<T>;
}

#include "detail/maybe.ipp"

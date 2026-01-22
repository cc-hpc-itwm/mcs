// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/Concepts.hpp>
#include <tuple>

namespace mcs::util::read
{
  template<is_readable... Ts>
    struct Read<std::tuple<Ts...>>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::tuple<Ts...>
        ;
  };
}

#include "detail/tuple.ipp"

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/Concepts.hpp>
#include <variant>

namespace mcs::util::read
{
  template<is_readable... Ts>
    struct Read<std::variant<Ts...>>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::variant<Ts...>
        ;
  };
}

#include "detail/variant.ipp"

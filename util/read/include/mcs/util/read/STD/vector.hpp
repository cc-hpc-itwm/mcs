// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/Concepts.hpp>
#include <vector>

namespace mcs::util::read
{
  template<is_readable T>
    struct Read<std::vector<T>>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::vector<T>
        ;
  };
}

#include "detail/vector.ipp"

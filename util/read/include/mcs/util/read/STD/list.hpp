// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <list>
#include <mcs/util/read/Concepts.hpp>

namespace mcs::util::read
{
  template<is_readable T>
    struct Read<std::list<T>>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::list<T>
        ;
  };
}

#include "detail/list.ipp"

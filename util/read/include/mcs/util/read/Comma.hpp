// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>

namespace mcs::util::read
{
  struct Comma{};

  template<>
    struct Read<Comma>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> Comma
        ;
  };
}

#include "detail/Comma.ipp"

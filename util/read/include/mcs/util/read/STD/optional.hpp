// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/Concepts.hpp>
#include <optional>

namespace mcs::util::read
{
  template<is_readable T>
    struct Read<std::optional<T>>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::optional<T>
        ;
  };
}

#include "detail/optional.ipp"

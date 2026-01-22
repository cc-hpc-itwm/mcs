// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>

namespace mcs::util::read
{
  template<>
    struct Read<std::filesystem::path>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::filesystem::path
        ;
  };
}

#include "detail/path.ipp"

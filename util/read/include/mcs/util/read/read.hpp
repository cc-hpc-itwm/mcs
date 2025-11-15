// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/util/read/State.hpp>

namespace mcs::util::read
{
  // Parses T from the input and requires no additional input after a
  // complete parse of T.
  //
  template<typename T, typename Char>
    [[nodiscard]] constexpr auto read (State<Char>&) -> T;

  // Create a state and then read T using read_from_state
  //
  template<typename T, typename... Args>
    [[nodiscard]] constexpr auto read (Args&&...) -> T;

  // Read the content of the file into memory and then parse is as T
  //
  template<typename T>
    [[nodiscard]] auto from_file (std::filesystem::path) -> T;
}

#include "detail/read.ipp"

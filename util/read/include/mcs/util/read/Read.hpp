// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util::read
{
  // Customization point. Define
  //
  //     template<typename Char>
  //       static auto Read<T>::read (State<Char>&) -> T;
  //
  template<typename T> struct Read;
}

// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <cstdint>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>

namespace mcs::util::read
{
  template<>
    struct Read<std::uint_least8_t>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::uint_least8_t
        ;
  };
  template<>
    struct Read<std::uint_least16_t>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::uint_least16_t
        ;
  };
  template<>
    struct Read<std::uint_least32_t>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::uint_least32_t
        ;
  };
  template<>
    struct Read<std::uint_least64_t>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::uint_least64_t
        ;
  };
  template<>
    struct Read<std::byte>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> std::byte
        ;
  };
}

#include "detail/uint.ipp"

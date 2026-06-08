// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/testing/random/value.hpp>
#include <mcs/testing/random/value/char.hpp>
#include <string>

namespace mcs::testing::random
{
  // produces random strings
  //
  template< class CharT
          , class Traits
          , class Allocator
          >
    struct value<std::basic_string<CharT, Traits, Allocator>>
  {
    using Type = std::basic_string<CharT, Traits, Allocator>;

    struct Length
    {
      typename Type::size_type min {0};
      typename Type::size_type max {1 << 20};
    };

    value() = default;
    explicit value (Length);

    // Arguments for the random_char generator
    //
    // EXAMPLE
    //
    //     mcs::testing::random::value<std::string> {'a', 'z'}
    //
    template<typename... Args> explicit value (Args&&...);
    template<typename... Args> explicit value (Length, Args&&...);

    auto operator()() -> Type;

  private:
    Length _length;
    value<CharT> _random_char;
  };
}

#include "detail/string.ipp"

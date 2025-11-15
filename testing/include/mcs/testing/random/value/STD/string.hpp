// Copyright (C) 2022-2025 Fraunhofer ITWM
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
  template<> struct value<std::string>
  {
    struct Length
    {
      std::string::size_type min {0};
      std::string::size_type max {1 << 20};
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

    auto operator()() -> std::string;

  private:
    Length _length;
    value<char> _random_char;
  };
}

#include "detail/string.ipp"

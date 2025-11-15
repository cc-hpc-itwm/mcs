// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/testing/random/value.hpp>

namespace mcs::testing::random
{
  template<typename T> struct value<std::vector<T>>
  {
    struct Length
    {
      std::vector<T>::size_type min {0};
      std::vector<T>::size_type max {1 << 20};
    };

    value() = default;
    explicit value (Length);

    // Arguments for the random value generator
    //
    // EXAMPLE
    //
    //     mcs::testing::random::value<std::vector<char>> {'a', 'z'}
    //
    template<typename... Args> explicit value (Args&&...);
    template<typename... Args> explicit value (Length, Args&&...);

    auto operator()() -> std::vector<T>;

  private:
    Length _length;
    value<T> _random_element;
  };
}

#include "detail/vector.ipp"

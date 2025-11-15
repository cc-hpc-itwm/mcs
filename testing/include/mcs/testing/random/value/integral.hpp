// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/testing/random/value.hpp>
#include <random>

namespace mcs::testing::random
{
  // produces random values from an interval
  //
  template<std::integral T>
    struct value<T>
  {
    struct Min { T value; };
    struct Max { T value; };

    value();
    explicit value (Min, Max);
    explicit value (T min, T max);
    explicit value (Min);
    explicit value (Max);

    auto operator()() -> T;

  private:
    struct UniformDistribution
    {
      struct MinMax
      {
        MinMax (T min, T max);

      private:
        friend struct UniformDistribution;
        T _min;
        T _max;
      };
      explicit UniformDistribution (MinMax);

      template<typename Generator>
        auto operator() (Generator&) -> T;

    private:
      std::uniform_int_distribution<T> _distribution;
    };
    UniformDistribution _value;
  };
}

#include "detail/integral.ipp"

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <functional>
#include <limits>
#include <mcs/testing/random/random_device.hpp>
#include <stdexcept>

namespace mcs::testing::random
{
  template<std::integral T>
    value<T>::UniformDistribution::MinMax::MinMax (T min, T max)
      : _min {min}
      , _max {max}
  {
    if (_min > _max)
    {
      throw std::invalid_argument
        {fmt::format ("value: min > max: {} > {}", _min, _max)};
    }
  }
  template<std::integral T>
    value<T>::UniformDistribution::UniformDistribution (MinMax min_max)
      : _distribution {min_max._min, min_max._max}
  {}
  template<std::integral T>
    template<typename Generator>
    auto value<T>::UniformDistribution::operator() (Generator& generator) -> T
  {
    return std::invoke (_distribution, generator);
  }

  template<std::integral T>
    value<T>::value (Min min, Max max)
      : value {min.value, max.value}
  {}
  template<std::integral T>
    value<T>::value (T min, T max)
      : _value {{min, max}}
  {}
  template<std::integral T>
    value<T>::value (Min min)
      : value {min.value, std::numeric_limits<T>::max()}
  {}
  template<std::integral T>
    value<T>::value (Max max)
      : value {std::numeric_limits<T>::lowest(), max.value}
  {}
  template<std::integral T>
    value<T>::value()
      : value { std::numeric_limits<T>::lowest()
              , std::numeric_limits<T>::max()
              }
  {}
  template<std::integral T>
    auto value<T>::operator()() -> T
  {
    return _value (random_device());
  }
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <iterator>
#include <mcs/testing/random/value/integral.hpp>

namespace mcs::testing::random
{
  template<typename T>
    template<typename... Args>
      value<std::vector<T>>::value (Args&&... args)
        : _random_element {std::forward<Args> (args)...}
  {}

  template<typename T>
    template<typename... Args>
      value<std::vector<T>>::value (Length length, Args&&... args)
        : _length {length}
        , _random_element {std::forward<Args> (args)...}
  {}

  template<typename T>
    value<std::vector<T>>::value (Length length)
      : _length {length}
  {
    if (_length.min > _length.max)
    {
      throw std::invalid_argument
        {"random::value<std:vector<T>>::Length: min > max"};
    }
  }

  template<typename T>
    auto value<std::vector<T>>::operator()() -> std::vector<T>
  {
    auto v {std::vector<T>{}};
    auto n {value<decltype (v.size())> {_length.min, _length.max}()};
    v.reserve (n);

    std::generate_n (std::back_inserter (v), n, _random_element);

    return v;
  }
}

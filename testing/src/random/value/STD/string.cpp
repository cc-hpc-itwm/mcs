// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/random/value/integral.hpp>

namespace mcs::testing::random
{
  value<std::string>::value (Length length)
    : _length {length}
  {
    if (_length.min > _length.max)
    {
      throw std::invalid_argument
        {"random::value<std:string>::Length: min > max"};
    }
  }

  auto value<std::string>::operator()() -> std::string
  {
    auto s {std::string{}};

    auto n {value<decltype (s.size())> {_length.min, _length.max}()};

    while (n --> 0)
    {
      s += _random_char();
    }

    return s;
  }
}

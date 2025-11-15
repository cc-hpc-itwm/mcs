// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/random/value/CIdent.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <stdexcept>

namespace mcs::testing::random
{
  value<CIdent>::value (Length length)
    : _length {length}
  {
    if (_length.min == 0)
    {
      throw std::invalid_argument {"CIdent can not have length zero"};
    }
    if (_length.min > _length.max)
    {
      throw std::invalid_argument {"random::value<CIdent>::Length: min > max"};
    }
  }

  auto value<CIdent>::operator()() -> std::string
  {
    auto s {std::string{}};

    auto n {value<decltype (s.size())> {_length.min, _length.max}()};

    s += _letters.at (_random_letter());

    while (n --> 0)
    {
      s += _chars.at (_random_char());
    }

    return s;
  }
}

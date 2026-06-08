// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <iterator>
#include <mcs/testing/random/value/integral.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::testing::random
{
  template< class CharT
          , class Traits
          , class Allocator
          >
    template<typename... Args>
      value<std::basic_string<CharT, Traits, Allocator>>::value (Args&&... args)
        : _random_char {std::forward<Args> (args)...}
  {}

  template< class CharT
          , class Traits
          , class Allocator
          >
    template<typename... Args>
      value<std::basic_string<CharT, Traits, Allocator>>::value
        ( Length length
        , Args&&... args
        )
          : _length {length}
          , _random_char {std::forward<Args> (args)...}
  {}

  template< class CharT
          , class Traits
          , class Allocator
          >
    value<std::basic_string<CharT, Traits, Allocator>>::value (Length length)
      : _length {length}
  {
    if (_length.min > _length.max)
    {
      throw std::invalid_argument
        { "random::value<std::basic_string<CharT, Traits, Allocator>>::"
          "Length: min > max"
        };
    }
  }

  template< class CharT
          , class Traits
          , class Allocator
          >
    auto value<std::basic_string<CharT, Traits, Allocator>>::operator()
      (
      ) -> Type
  {
    auto result {Type{}};
    auto length {value<decltype (result.size())> {_length.min, _length.max}()};
    result.reserve (length);

    std::generate_n (std::back_inserter (result), length, _random_char);

    return result;
  }
}

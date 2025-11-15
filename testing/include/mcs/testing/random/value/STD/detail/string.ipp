// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::testing::random
{
  template<typename... Args>
    value<std::string>::value (Args&&... args)
      : _random_char {std::forward<Args> (args)...}
  {}

  template<typename... Args>
    value<std::string>::value (Length length, Args&&... args)
      : _length {length}
      , _random_char {std::forward<Args> (args)...}
  {}
}

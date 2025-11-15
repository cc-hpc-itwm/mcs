// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/testing/random/value.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <string>

namespace mcs::testing::random
{
  struct CIdent{};

  // produces random c identifiers
  //
  template<> struct value<CIdent>
  {
    struct Length
    {
      std::string::size_type min {1};
      std::string::size_type max {1 << 6};
    };

    value() = default;
    explicit value (Length);

    auto operator()() -> std::string;

  private:
    Length _length;
    std::string const _letters
      {"_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    std::string const _chars
      {"0123456789" + _letters};
    value<std::string::size_type> _random_letter {0, 2 * 26 + 1 - 1};
    value<std::string::size_type> _random_char {0, 10 + 2 * 26 + 1 - 1};
  };
}

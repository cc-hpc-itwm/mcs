// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/State.hpp>

namespace mcs::util::read
{
  // Parser for a "symbol", that is: One or more characters.
  //
  // EXAMPLE:
  //
  //     auto bracket {symbol ("(")};
  //
  template<typename What>
    struct Symbol
  {
    constexpr explicit Symbol (What) noexcept;

    template<typename Char>
      constexpr auto operator() (State<Char>&) const;

  private:
    What _what;
  };

  template<typename What>
    constexpr auto symbol (What) noexcept;
}

#include "detail/Symbol.ipp"

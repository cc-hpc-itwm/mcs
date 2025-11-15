// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <stdexcept>

namespace mcs::util::read
{
  template<typename Allowed, typename Char>
    constexpr auto one_of (State<Char>& state, Allowed _allowed) -> Char
  {
    auto allowed {make_state (_allowed)};

    while (!allowed.end() && !state.end())
    {
      if (allowed.head() == state.peek())
      {
        return state.head();
      }
    }

    throw state.error (fmt::format ("Expected one of '{}'.", _allowed));
  }
}

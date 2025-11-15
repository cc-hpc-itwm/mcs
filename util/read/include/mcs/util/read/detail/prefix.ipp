// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/util/read/skip_whitespace.hpp>
#include <stdexcept>

namespace mcs::util::read
{
  template<typename Prefix, typename Char>
    constexpr auto prefix (State<Char>& state, Prefix _prefix) -> State<Char>&
  {
    skip_whitespace (state);

    auto prefix {make_state (_prefix)};

    while (!prefix.end())
    {
      if (state.end() || prefix.head() != state.peek())
      {
        throw state.error (fmt::format ("Expected '{}'.", _prefix));
      }

      state.skip();
    }

    return state;
  }

  template<typename Prefix, typename Char>
    constexpr auto maybe_prefix (State<Char>& state, Prefix _prefix) -> bool
  {
   auto const saved_state {state};

    skip_whitespace (state);

    auto prefix {make_state (_prefix)};

    while (!prefix.end())
    {
      if (state.end() || prefix.head() != state.peek())
      {
        state = saved_state;

        return false;
      }

      state.skip();
    }

    return true;
  }
}

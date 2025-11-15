// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/isspace.hpp>

namespace mcs::util::read
{
  template<typename Char>
    constexpr auto skip_whitespace (State<Char>& state) -> void
  {
    while (!state.end() && isspace (state.peek()))
    {
      state.skip();
    }
  }
}

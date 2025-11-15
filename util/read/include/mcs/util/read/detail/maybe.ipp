// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/parse.hpp>

namespace mcs::util::read
{
  template<typename T, typename Char>
    constexpr auto maybe (State<Char>& state) -> std::optional<T>
  {
    auto const saved_state {state};

    try
    {
      return parse<T> (state);
    }
    catch (...)
    {
      state = saved_state;

      return {};
    }
  }
}

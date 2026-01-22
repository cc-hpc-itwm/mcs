// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/STD/tuple.hpp>
#include <mcs/util/read/parse.hpp>
#include <mcs/util/read/prefix.hpp>
#include <utility>

namespace mcs::util::read
{
  template<is_readable T>
    template<typename Char>
      auto Read<std::optional<T>>::read
        ( State<Char>& state
        ) -> std::optional<T>
  {
    if (maybe_prefix (state, "Nothing"))
    {
      return {};
    }

    prefix (state, "Just");

    auto [value] {parse<std::tuple<T>> (state)};

    return std::move (value);
  }
}

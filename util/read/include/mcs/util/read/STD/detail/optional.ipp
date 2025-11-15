// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/STD/tuple.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/parse.hpp>
#include <mcs/util/read/prefix.hpp>
#include <utility>

namespace mcs::util::read
{
  template<is_readable T>
    MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, std::optional<T>)
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

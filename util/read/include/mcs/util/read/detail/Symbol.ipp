// Copyright (C) 2022,2024-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/skip_whitespace.hpp>
#include <utility>

namespace mcs::util::read
{
  template<typename What>
    constexpr Symbol<What>::Symbol (What what) noexcept
      : _what {std::move (what)}
  {}

  template<typename What>
    template<typename Char>
      constexpr auto Symbol<What>::operator() (State<Char>& state) const
  {
    skip_whitespace (state);
    prefix (state, _what);
  }

  template<typename What>
    constexpr auto symbol (What what) noexcept
  {
    return Symbol<What> {what};
  }
}

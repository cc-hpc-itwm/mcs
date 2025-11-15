// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <iterator>
#include <mcs/util/read/Comma.hpp>
#include <mcs/util/read/maybe.hpp>

namespace mcs::util::read::detail
{
  template<is_readable T, template<typename...> typename Container>
    struct RangeElements
  {
    template<typename Char>
      [[nodiscard]] constexpr auto operator()
        ( State<Char>& state
        ) const
    {
      auto xs {Container<T>{}};

      while (auto x {maybe<T> (state)})
      {
        xs.emplace (std::end (xs), std::move (x.value()));
        std::ignore = maybe<Comma> (state);
      }

      return xs;
    }
  };
}

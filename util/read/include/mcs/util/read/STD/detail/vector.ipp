// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/InBrackets.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/Symbol.hpp>
#include <mcs/util/read/detail/RangeElements.hpp>

namespace mcs::util::read
{
  template<is_readable T>
    template<typename Char>
      auto Read<std::vector<T>>::read
        ( State<Char>& state
        ) -> std::vector<T>
  {
    return in_brackets
      ( symbol ("[")
      , detail::RangeElements<T, std::vector>{}
      , symbol ("]")
      ) (state)
      ;
  }
}

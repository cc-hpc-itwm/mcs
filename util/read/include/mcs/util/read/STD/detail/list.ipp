// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/InBrackets.hpp>
#include <mcs/util/read/Symbol.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/detail/RangeElements.hpp>

namespace mcs::util::read
{
  template<is_readable T>
    MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, std::list<T>)
  {
    return in_brackets
      ( symbol ("[")
      , detail::RangeElements<T, std::list>{}
      , symbol ("]")
      ) (state)
      ;
  }
}

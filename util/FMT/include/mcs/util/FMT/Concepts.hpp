// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>

namespace mcs::util::FMT
{
  // The concepts sink specifies that there is an overload of
  // fmt::print that takes a `Sink` as the first argument.
  //
  // fmt provides overloads for `FILE*` and `std::ostream&`.
  //
  template<typename Sink>
    concept sink = requires (Sink sink)
      {
        fmt::print (sink, "");
      }
  ;
}

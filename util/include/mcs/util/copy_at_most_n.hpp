// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util
{
  // Copies at most count many values from the range [first, last) to
  // the range beginning at result.
  //
  // If n = std::distance (first, last), then std::min (count, n)
  // values are copied.
  //
  template<class InputIt, class Size, class OutputIt>
    constexpr auto copy_at_most_n
      ( InputIt first, InputIt last
      , Size count
      , OutputIt result
      ) -> OutputIt
    ;
}

#include "detail/copy_at_most_n.ipp"

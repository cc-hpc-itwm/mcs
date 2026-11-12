// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <string_view>

namespace mcs::util::read
{
  struct SeparatedFields
  {
    std::string_view prefix;
    std::string_view suffix;
  };

  // Split at the last whitespace. Earlier whitespace stays in the
  // prefix. Both fields must be nonempty. Fields borrow the input,
  // which must outlive their use.
  //
  [[nodiscard]] auto split_at_last_whitespace
    ( std::string_view
    ) -> SeparatedFields
    ;
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE
    ( context
    , mcs::core::storage::trace::event::size::used::Result
    )
  {
    return context.begin();
  }

  MCS_UTIL_FMT_DEFINE_FORMAT
    ( size_used_result
    , context
    , mcs::core::storage::trace::event::size::used::Result
    )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::size::used::Result = {}"
      , size_used_result._size_used
      );
  }
}

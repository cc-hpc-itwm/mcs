// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE
    ( context
    , mcs::core::storage::trace::event::size::max::Result
    )
  {
    return context.begin();
  }

  MCS_UTIL_FMT_DEFINE_FORMAT
    ( size_max_result
    , context
    , mcs::core::storage::trace::event::size::max::Result
    )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::size::max::Result = {}"
      , size_max_result._max_size
      );
  }
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE
    ( context
    , mcs::core::storage::trace::event::segment::remove::Result
    )
  {
    return context.begin();
  }

  MCS_UTIL_FMT_DEFINE_FORMAT
    ( segment_remove_result
    , context
    , mcs::core::storage::trace::event::segment::remove::Result
    )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::segment::remove::Result = {}"
      , segment_remove_result._size
      );
  }
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE
    ( context
    , mcs::core::storage::trace::event::segment::create::Result
    )
  {
    return context.begin();
  }

  MCS_UTIL_FMT_DEFINE_FORMAT
    ( segment_create_result
    , context
    , mcs::core::storage::trace::event::segment::create::Result
    )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::segment::create::Result = {}"
      , segment_create_result._segment_id
      );
  }
}

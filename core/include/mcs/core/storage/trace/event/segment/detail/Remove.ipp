// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/FMT/define.hpp>
#include <tuple>

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::trace::event::segment::Remove<Storage>
      )
  {
    return context.begin();
  }

  template<mcs::core::storage::is_implementation Storage>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( segment_remove
      , context
      , mcs::core::storage::trace::event::segment::Remove<Storage>
      )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::segment::Remove {}"
      , std::make_tuple
        ( segment_remove._parameter_segment_remove
        , segment_remove._segment_id
        )
      );
  }
}

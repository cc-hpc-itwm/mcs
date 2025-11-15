// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE
    ( context
    , mcs::core::storage::trace::event::file::write::Result
    )
  {
    return context.begin();
  }

  MCS_UTIL_FMT_DEFINE_FORMAT
    ( file_write
    , context
    , mcs::core::storage::trace::event::file::write::Result
    )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::file::write::Result = {}"
      , file_write._size
      );
  }
}

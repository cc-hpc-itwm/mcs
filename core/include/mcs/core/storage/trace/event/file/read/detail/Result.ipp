// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE
    ( context
    , mcs::core::storage::trace::event::file::read::Result
    )
  {
    return context.begin();
  }

  MCS_UTIL_FMT_DEFINE_FORMAT
    ( file_read
    , context
    , mcs::core::storage::trace::event::file::read::Result
    )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::file::read::Result = {}"
      , file_read._size
      );
  }
}

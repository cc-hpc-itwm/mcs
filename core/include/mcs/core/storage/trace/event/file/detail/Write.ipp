// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/FMT/define.hpp>
#include <tuple>

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::trace::event::file::Write<Storage>
      )
  {
    return context.begin();
  }

  template<mcs::core::storage::is_implementation Storage>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( file_write
      , context
      , mcs::core::storage::trace::event::file::Write<Storage>
      )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::file::Write {}"
      , std::make_tuple
        ( file_write._parameter_file_write
        , file_write._segment_id
        , file_write._offset
        , file_write._path
        , file_write._range
        )
      );
  }
}

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
      , mcs::core::storage::trace::event::file::Read<Storage>
      )
  {
    return context.begin();
  }

  template<mcs::core::storage::is_implementation Storage>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( file_read
      , context
      , mcs::core::storage::trace::event::file::Read<Storage>
      )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::file::Read {}"
      , std::make_tuple
        ( file_read._parameter_file_read
        , file_read._segment_id
        , file_read._offset
        , file_read._path
        , file_read._range
        )
      );
  }
}

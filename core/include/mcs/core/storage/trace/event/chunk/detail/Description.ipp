// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/FMT/define.hpp>
#include <tuple>

namespace fmt
{
  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::trace::event::chunk::Description<Storage, Access>
      )
  {
    return context.begin();
  }

  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( chunk_description
      , context
      , mcs::core::storage::trace::event::chunk::Description<Storage, Access>
      )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::chunk::Description<{}> {}"
      , Access{}
      , std::make_tuple
        ( chunk_description._parameter_chunk_description
        , chunk_description._segment_id
        , chunk_description._memory_range
        )
      );
  }
}

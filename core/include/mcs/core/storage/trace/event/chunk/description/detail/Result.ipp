// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::trace::event::chunk::description
          ::Result<Storage, Access>
      )
  {
    return context.begin();
  }

  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( chunk_description_result
      , context
      , mcs::core::storage::trace::event::chunk::description
          ::Result<Storage, Access>
      )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::chunk::description::Result<{}> = {}"
      , Access{}
      , chunk_description_result._chunk_description
      );
  }
}

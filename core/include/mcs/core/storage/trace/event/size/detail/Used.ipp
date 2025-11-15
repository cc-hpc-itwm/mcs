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
      , mcs::core::storage::trace::event::size::Used<Storage>
      )
  {
    return context.begin();
  }

  template<mcs::core::storage::is_implementation Storage>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( size_used
      , context
      , mcs::core::storage::trace::event::size::Used<Storage>
      )
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::size::Used {}"
      , std::make_tuple
        ( size_used._parameter_size_used
        )
      );
  }
}

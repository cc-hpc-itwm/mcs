// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE
    ( context
    , mcs::core::storage::tracer::log_file::Tag
    )
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT
    ( tag
    , context
    , mcs::core::storage::tracer::log_file::Tag
    )
  {
    std::ignore = tag;

    return fmt::format (context.out(), "tracer::LogFile");
  }
}

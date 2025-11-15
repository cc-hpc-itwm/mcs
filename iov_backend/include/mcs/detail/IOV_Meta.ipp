// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/FMT/define.hpp>
#include <tuple>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (context, iov::meta::Name)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (name, context, iov::meta::Name)
  {
    return fmt::format_to
      ( context.out()
      , "iov::meta::Name {}"
      , std::make_tuple (name.str())
      );
  }
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (context, iov::meta::Key)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (key, context, iov::meta::Key)
  {
    return fmt::format_to
      ( context.out()
      , "iov::meta::Key {}"
      , std::make_tuple (key.str())
      );
  }
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (context, iov::meta::Value)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (value, context, iov::meta::Value)
  {
    return fmt::format_to
      ( context.out()
      , "iov::meta::Value {}"
      , std::make_tuple (value.str())
      );
  }
}

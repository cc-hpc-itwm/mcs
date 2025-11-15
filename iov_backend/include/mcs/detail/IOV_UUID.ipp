// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (context, iov::meta::UUID)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (uuid, context, iov::meta::UUID)
  {
    return fmt::format_to
      ( context.out()
      , "iov::meta::UUID {}"
      , std::make_tuple (uuid.str())
      );
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, iov::meta::UUID)
  {
    prefix (state, "iov::meta::UUID");

    return std::make_from_tuple<iov::meta::UUID>
      ( parse<std::tuple<iov::meta::UUID_FingerPrint>> (state)
      );
  }
}

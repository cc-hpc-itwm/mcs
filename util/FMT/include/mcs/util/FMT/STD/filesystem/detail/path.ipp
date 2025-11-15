// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
#include <mcs/util/string.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, std::filesystem::path)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (path, ctx, std::filesystem::path)
  {
    return fmt::format_to (ctx.out(), "{}", mcs::util::string {path.string()});
  }
}

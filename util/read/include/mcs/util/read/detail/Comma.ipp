// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/define.hpp>
#include <mcs/util/read/prefix.hpp>

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, Comma)
  {
    skip_whitespace (state);
    prefix (state, ",");
    return Comma{};
  }
}

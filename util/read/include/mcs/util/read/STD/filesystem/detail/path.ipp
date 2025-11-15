// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/define.hpp>
#include <mcs/util/read/parse.hpp>
#include <mcs/util/string.hpp>

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    ( state
    , std::filesystem::path
    )
  {
    return std::filesystem::path {parse<util::string> (state)};
  }
}

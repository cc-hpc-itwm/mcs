// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/util/FMT/declare.hpp>

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (std::filesystem::path);
}

#include "detail/path.ipp"

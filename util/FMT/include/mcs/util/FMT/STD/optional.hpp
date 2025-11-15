// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/FMT/declare.hpp>
#include <optional>

namespace fmt
{
  template<formattable T>
    MCS_UTIL_FMT_DECLARE (std::optional<T>);
}

#include "detail/optional.ipp"

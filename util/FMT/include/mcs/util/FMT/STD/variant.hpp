// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/FMT/declare.hpp>
#include <variant>

namespace fmt
{
  template<formattable... Ts>
    MCS_UTIL_FMT_DECLARE (std::variant<Ts...>);
}

#include "detail/variant.ipp"

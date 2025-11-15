// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/declare.hpp>
#include <variant>

namespace mcs::util::read
{
  template<is_readable... Ts>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION (std::variant<Ts...>);
}

#include "detail/variant.ipp"

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/declare.hpp>
#include <optional>

namespace mcs::util::read
{
  template<is_readable T>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION (std::optional<T>);
}

#include "detail/optional.ipp"

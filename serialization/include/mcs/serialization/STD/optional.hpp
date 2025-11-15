// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/declare.hpp>
#include <optional>

namespace mcs::serialization
{
  template<is_serializable T>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (std::optional<T>)
    ;
}

#include "detail/optional.ipp"

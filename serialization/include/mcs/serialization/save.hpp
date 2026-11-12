// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <type_traits>

namespace mcs::serialization
{
  template<is_serializable T>
    constexpr auto save (OArchive&, T const&) -> OArchive&;
}

#include "detail/save.ipp"

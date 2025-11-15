// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <type_traits>

namespace mcs::serialization
{
  template<is_serializable T>
    constexpr auto save (OArchive&, T const&) -> OArchive&;
  template<is_serializable T>
    constexpr auto save (OArchive&, T&) -> OArchive&;
  template<is_serializable T>
    auto save (OArchive&, T&&) -> OArchive&;
}

#include "detail/save.ipp"

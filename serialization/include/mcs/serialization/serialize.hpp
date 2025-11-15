// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  template<is_serializable T>
    constexpr auto operator& (IArchive&, T&) -> IArchive&;

  template<is_serializable T>
    constexpr auto operator& (IArchive&, T&&) -> IArchive& = delete;

  template<is_serializable T>
    constexpr auto operator& (OArchive&, T const&) -> OArchive&;
  template<is_serializable T>
    constexpr auto operator& (OArchive&, T& x) -> OArchive&;

  template<is_empty T>
    constexpr auto operator& (OArchive&, T&&) -> OArchive&;
  template<is_serializable T>
    requires (!is_empty<T>)
    constexpr auto operator& (OArchive&, T&&) -> OArchive& = delete;
}

#include "detail/serialize.ipp"

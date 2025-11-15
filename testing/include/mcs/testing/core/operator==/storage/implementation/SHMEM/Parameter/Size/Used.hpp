// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <type_traits>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( SHMEM::Parameter::Size::Used const&
    , SHMEM::Parameter::Size::Used const&
    ) noexcept -> bool
  {
    static_assert (std::is_empty_v<SHMEM::Parameter::Size::Used>);

    return true;
  }
}

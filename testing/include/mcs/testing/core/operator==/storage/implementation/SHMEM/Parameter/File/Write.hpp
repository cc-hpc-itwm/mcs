// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <type_traits>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( SHMEM::Parameter::File::Write const&
    , SHMEM::Parameter::File::Write const&
    ) noexcept -> bool
  {
    static_assert (std::is_empty_v<SHMEM::Parameter::File::Write>);

    return true;
  }
}

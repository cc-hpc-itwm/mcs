// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/SHMEM.hpp>

namespace mcs::core::storage::implementation
{
  auto operator==
    ( typename SHMEM::Prefix const&
    , typename SHMEM::Prefix const&
    ) noexcept -> bool;
}

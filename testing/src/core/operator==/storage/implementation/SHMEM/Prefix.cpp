// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/operator==/storage/implementation/SHMEM/Prefix.hpp>

namespace mcs::core::storage::implementation
{
  auto operator==
    ( typename SHMEM::Prefix const& lhs
    , typename SHMEM::Prefix const& rhs
    ) noexcept -> bool
  {
    return lhs.value == rhs.value;
  }
}

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/random/storage/implementation/SHMEM/Prefix.hpp>
#include <mcs/testing/random/value/define.hpp>

namespace mcs::testing::random
{
  auto value<core::storage::implementation::SHMEM::Prefix>::operator()
    (
    ) -> core::storage::implementation::SHMEM::Prefix
  {
    return core::storage::implementation::SHMEM::Prefix
      {util::string {_string()}};
  }
}

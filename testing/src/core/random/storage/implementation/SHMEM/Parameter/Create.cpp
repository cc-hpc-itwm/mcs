// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/random/storage/implementation/SHMEM/Parameter/Create.hpp>
#include <mcs/testing/random/value/define.hpp>

namespace mcs::testing::random
{
  using Create = core::storage::implementation::SHMEM::Parameter::Create;

  auto value<Create>::operator()() -> Create
  {
    return Create {_prefix(), _max_size()};
  }
}

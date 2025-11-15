// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/random/value/STD/string.hpp>

namespace mcs::testing::random
{
  template<> struct value<core::storage::implementation::SHMEM::Prefix>
  {
    auto operator()() -> core::storage::implementation::SHMEM::Prefix;

  private:
    value<std::string> _string;
  };
}

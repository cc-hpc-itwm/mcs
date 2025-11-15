// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/storage/implementation/Virtual.hpp>

namespace mcs::testing::core::storage::implementation
{
  auto SharedObject<Files>::path() -> std::filesystem::path
  {
    return TESTING_MCS_CORE_STORAGE_IMPLEMENT_C_API_FILES;
  }
  auto SharedObject<Heap>::path() -> std::filesystem::path
  {
    return TESTING_MCS_CORE_STORAGE_IMPLEMENT_C_API_HEAP;
  }
  auto SharedObject<SHMEM>::path() -> std::filesystem::path
  {
    return TESTING_MCS_CORE_STORAGE_IMPLEMENT_C_API_SHMEM;
  }
}

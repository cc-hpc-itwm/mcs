// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/implementation/Import_C_API.hpp>

namespace mcs::core::storage::implementation
{
  TEST (MCSStorageImport_C_API, is_a_storage_implementation)
  {
    static_assert (is_implementation<Import_C_API>);
  }
}

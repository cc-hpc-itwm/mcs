// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <mcs/core/storage/Implement_C_API.hpp>
#include <mcs/core/storage/c_api.h>
#include <mcs/core/storage/implementation/Files.hpp>

extern "C" auto mcs_core_storage_methods() -> struct ::mcs_core_storage
{
  return std::invoke
    ( mcs::core::storage::Implement_C_API::methods
        < mcs::core::storage::implementation::Files
        >
    );
}

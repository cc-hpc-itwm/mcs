// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/util/type/List.hpp>

namespace
{
  using SupportedStorageImplementations = mcs::util::type::List
    < mcs::core::storage::implementation::Files
    , mcs::core::storage::implementation::SHMEM
    >;
}

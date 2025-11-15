// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::iov_backend
{
  using SupportedStorageImplementations = util::type::List
    < core::storage::implementation::Files
    , core::storage::implementation::Heap
    , core::storage::implementation::SHMEM
    >
    ;

  template<typename I>
    concept is_supported_storage_implementation =
      SupportedStorageImplementations::template contains<I>()
    ;
}

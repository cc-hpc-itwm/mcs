// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/chunk/Description.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/declare.hpp>

namespace mcs::core::control::command::chunk
{
  template< core::chunk::is_access Access
          , core::storage::is_implementation... StorageImplementations
          >
    struct Description
  {
    using Response
      = core::chunk::Description<Access, StorageImplementations...>
      ;

    core::storage::ID storage_id;
    core::storage::Parameter storage_parameter;
    core::storage::segment::ID segment_id;
    core::memory::Range memory_range;
  };
}

namespace mcs::serialization
{
  template< core::chunk::is_access Access
          , core::storage::is_implementation... StorageImplementations
          >
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (core::control::command::chunk::Description<Access, StorageImplementations...>);
}

#include "detail/Description.ipp"

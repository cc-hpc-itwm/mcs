// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::trace::event::chunk::description
{
  template< core::storage::is_implementation Storage
          , core::chunk::is_access Access
          >
    struct Result
  {
    typename Storage::Chunk::template Description<Access> _chunk_description;
  };
}

namespace fmt
{
  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::trace::event::chunk::description
          ::Result<Storage, Access>
      );
}

#include "detail/Result.ipp"

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::trace::event::chunk
{
  template< core::storage::is_implementation Storage
          , core::chunk::is_access Access
          >
    struct Description
  {
    typename Storage::Parameter::Chunk::Description
      _parameter_chunk_description;
    core::storage::segment::ID _segment_id;
    core::memory::Range _memory_range;
  };
}

namespace fmt
{
  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::trace::event::chunk::Description<Storage, Access>
      );
}

#include "detail/Description.ipp"

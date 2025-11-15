// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::trace::event::segment
{
  template<core::storage::is_implementation Storage>
    struct Remove
  {
    typename Storage::Parameter::Segment::Remove _parameter_segment_remove;
    core::storage::segment::ID _segment_id;
  };
}

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::trace::event::segment::Remove<Storage>
      );
}

#include "detail/Remove.ipp"

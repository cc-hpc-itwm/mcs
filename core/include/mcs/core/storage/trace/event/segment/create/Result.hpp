// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::trace::event::segment::create
{
  struct Result
  {
    core::storage::segment::ID _segment_id;
  };
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::trace::event::segment::create::Result
      );
}

#include "detail/Result.ipp"

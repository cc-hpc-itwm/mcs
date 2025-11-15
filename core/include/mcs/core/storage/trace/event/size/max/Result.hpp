// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::trace::event::size::max
{
  struct Result
  {
    core::storage::MaxSize _max_size;
  };
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::trace::event::size::max::Result
      );
}

#include "detail/Result.ipp"

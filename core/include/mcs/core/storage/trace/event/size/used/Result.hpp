// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Size.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::trace::event::size::used
{
  struct Result
  {
    core::memory::Size _size_used;
  };
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::trace::event::size::used::Result
      );
}

#include "detail/Result.ipp"

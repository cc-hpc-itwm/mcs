// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/Concepts.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::trace::event::size
{
  template<core::storage::is_implementation Storage>
    struct Max
  {
    typename Storage::Parameter::Size::Max _parameter_size_max;
  };
}

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::trace::event::size::Max<Storage>
      );
}

#include "detail/Max.ipp"

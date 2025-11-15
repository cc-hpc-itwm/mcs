// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::trace::event::segment
{
  template<core::storage::is_implementation Storage>
    struct Create
  {
    typename Storage::Parameter::Segment::Create _parameter_segment_create;
    core::memory::Size _size;
  };
}

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::trace::event::segment::Create<Storage>
      );
}

#include "detail/Create.ipp"

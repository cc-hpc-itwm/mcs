// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/Concepts.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::implementation::trace
{
  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    struct Tag{};
}

namespace fmt
{
  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::implementation::trace::Tag<Tracer, Storage>
      );
}

#include "detail/Tag.ipp"

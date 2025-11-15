// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::tracer::record
{
  struct Tag{};
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE (mcs::core::storage::tracer::record::Tag);
}

static_assert
  ( mcs::serialization::is_serializable
      < mcs::core::storage::tracer::record::Tag
      >
  );

#include "detail/Tag.ipp"

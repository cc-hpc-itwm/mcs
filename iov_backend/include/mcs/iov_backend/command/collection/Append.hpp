// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/serialization/declare.hpp>

namespace mcs::iov_backend::command::collection
{
  struct Append
  {
    using Response = core::memory::Size;

    iov_backend::collection::ID _collection_id;
    core::memory::Range _range_to_append;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( iov_backend::command::collection::Append
      );
}

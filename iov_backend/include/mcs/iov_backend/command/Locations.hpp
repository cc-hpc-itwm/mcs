// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Range.hpp>
#include <mcs/iov_backend/Locations.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/serialization/declare.hpp>

namespace mcs::iov_backend::command
{
  struct Locations
  {
    using Response = iov_backend::Locations;

    collection::ID _collection_id;
    core::memory::Range _range;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( iov_backend::command::Locations
      );
}

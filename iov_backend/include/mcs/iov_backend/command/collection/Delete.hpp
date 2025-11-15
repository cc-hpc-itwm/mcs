// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/serialization/declare.hpp>

namespace mcs::iov_backend::command::collection
{
  struct Delete
  {
    using Response = void;

    iov_backend::collection::ID _collection_id;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( iov_backend::command::collection::Delete
      );
}

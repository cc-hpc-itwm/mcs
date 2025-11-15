// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Range.hpp>
#include <mcs/iov_backend/Storage.hpp>
#include <mcs/iov_backend/storage/ID.hpp>
#include <mcs/serialization/declare.hpp>

namespace mcs::iov_backend::command::storage
{
  struct Add
  {
    using Response = iov_backend::storage::ID;

    Storage storage;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( iov_backend::command::storage::Add
      );
}

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/iov_backend/provider/State.hpp>
#include <mcs/serialization/declare.hpp>

namespace mcs::iov_backend::command
{
  struct State
  {
    using Response = iov_backend::provider::State;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( iov_backend::command::State
      );
}

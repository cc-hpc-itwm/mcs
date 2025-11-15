// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>

namespace mcs::rpc::error
{
  // During construction the client shakes hand with the provider and
  // verifies that it supports the _exact same set_ of commands as the
  // dispatcher, in the _exact same order_.
  //
  struct HandshakeFailed : public mcs::Error
  {
  public:
    HandshakeFailed() noexcept;

    MCS_ERROR_COPY_MOVE_DEFAULT (HandshakeFailed);
  };
}

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/error/HandshakeFailed.hpp>

namespace mcs::rpc::error
{
  HandshakeFailed::HandshakeFailed() noexcept
    : mcs::Error
      {"rpc::error::Handshake: Failed. Clients must support a prefix"
       " of the commands that the dispatcher of the provider provides."
      }
  {}
  HandshakeFailed::~HandshakeFailed() = default;
}

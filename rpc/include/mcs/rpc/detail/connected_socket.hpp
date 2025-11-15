// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/Concepts.hpp>
#include <memory>

namespace mcs::rpc::detail
{
  // Connects, sets socket options and shakes hand with the other side.
  //
  template< is_protocol Protocol
          , is_command... Commands
          , typename Executor
          >
    auto connected_socket
      ( Executor&
      , typename Protocol::endpoint
      ) -> std::shared_ptr<typename Protocol::socket>
    ;
}

#include "detail/connected_socket.ipp"

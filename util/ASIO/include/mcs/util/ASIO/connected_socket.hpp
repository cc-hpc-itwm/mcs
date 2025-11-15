// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>

namespace mcs::util::ASIO
{
  template<is_protocol Protocol, typename Executor>
    auto connected_socket
      ( Executor&
      , typename Protocol::endpoint
      ) -> typename Protocol::socket
    ;

  template<is_protocol Protocol, typename Executor>
    auto connected_socket
      ( Executor&
      , Connectable<Protocol>
      ) -> typename Protocol::socket
    ;
}

#include "detail/connected_socket.ipp"

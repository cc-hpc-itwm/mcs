// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <mcs/util/read/declare.hpp>
#include <variant>

namespace mcs::util::ASIO
{
  using Endpoint = std::variant
      < asio::ip::tcp::endpoint
      , asio::local::stream_protocol::endpoint
      >;

  template<typename Runner, typename... Args>
    auto run (Endpoint, Runner&&, Args&&...);
}

namespace mcs::util::read
{
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( asio::ip::tcp::endpoint
      );
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( asio::local::stream_protocol::endpoint
      );
}

#include "detail/Endpoint.ipp"

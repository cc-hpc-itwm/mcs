// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <type_traits>

namespace mcs::util::ASIO
{
  template<typename Protocol>
    concept is_protocol =
       std::is_same_v<Protocol, asio::ip::tcp>
    || std::is_same_v<Protocol, asio::local::stream_protocol>
    ;
}

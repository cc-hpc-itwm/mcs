// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>

namespace mcs::util::ASIO
{
  template<is_protocol Protocol> struct SetSocketOptions;

  template<> struct SetSocketOptions<asio::local::stream_protocol>
  {
    constexpr auto operator()
      ( asio::local::stream_protocol::socket&
      ) const noexcept -> void
      ;
  };

  template<> struct SetSocketOptions<asio::ip::tcp>
  {
    auto operator()
      ( asio::ip::tcp::socket&
      ) const -> void
      ;
  };
}

#include "detail/SetSocketOptions.ipp"

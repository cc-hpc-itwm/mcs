// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
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
    struct Read<asio::ip::tcp::endpoint>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> asio::ip::tcp::endpoint
        ;
  };
  template<>
    struct Read<asio::local::stream_protocol::endpoint>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> asio::local::stream_protocol::endpoint
        ;
  };
}

#include "detail/Endpoint.ipp"

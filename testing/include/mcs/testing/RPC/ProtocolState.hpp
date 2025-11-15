// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <string>

namespace mcs::testing::RPC
{
  template<typename> struct ProtocolState;

  template<> struct ProtocolState<asio::ip::tcp>
  {
    ProtocolState()
      : ProtocolState {configured_ip_version(), {}}
    {}

    ProtocolState (std::string tag) noexcept
      : ProtocolState {configured_ip_version(), tag}
    {}
    template<typename TCPVersion>
      ProtocolState (TCPVersion tcp_version, std::string) noexcept
        : _endpoint {tcp_version, 0}
    {}

    [[nodiscard]] auto local_endpoint
      (
      ) const -> asio::ip::tcp::endpoint const&
      ;

  private:
    // Returns one of {`asio::ip::tcp::v4()`, `asio::ip::tcp::v6()`}.
    // Depending on the value of `MCS_TESTING_RPC_IP_VERSION`, which has to
    // be set as a compile definition.
    //
    [[nodiscard]] static auto configured_ip_version() noexcept
      -> asio::ip::tcp;
    asio::ip::tcp::endpoint _endpoint;
  };

  template<> struct ProtocolState<asio::local::stream_protocol>
  {
    ProtocolState()
      : _temp {"TESTING-RPC-PROTOCOLSTATE"}
    {}

    ProtocolState (std::string tag)
      : _temp {fmt::format ("TESTING-RPC-PROTOCOLSTATE-{}", tag)}
    {}

    // avoid default constructor as it would introduce NUL characters
    // they do not work well together with std::system
    //
    [[nodiscard]] auto local_endpoint
      (
      ) const -> asio::local::stream_protocol::endpoint const&
      ;

  private:
    UniqTemporaryDirectory _temp;
    std::filesystem::path _path {_temp.path() / "SOCK"};
    asio::local::stream_protocol::endpoint _endpoint {_path.native()};
  };
}

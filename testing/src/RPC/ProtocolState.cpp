// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/RPC/ProtocolState.hpp>

namespace mcs::testing::RPC
{
  auto ProtocolState<asio::ip::tcp>::configured_ip_version
    (
    ) noexcept (  noexcept (asio::ip::tcp::v4())
               && noexcept (asio::ip::tcp::v6())
               )
    -> asio::ip::tcp
  {
#   if (MCS_TESTING_RPC_IP_VERSION == 4)
      return {asio::ip::tcp::v4()};
#   elif (MCS_TESTING_RPC_IP_VERSION == 6)
      return {asio::ip::tcp::v6()};
#   else
#     error "MCS_TESTING_RPC_IP_VERSION is not supported. Supported versions are: {'4', '6'}."
      __builtin_unreachable();
#   endif
  }
  auto ProtocolState<asio::ip::tcp>::local_endpoint
    (
    ) const -> asio::ip::tcp::endpoint const&
  {
    return _endpoint;
  }
  auto ProtocolState<asio::local::stream_protocol>::local_endpoint
    (
    ) const -> asio::local::stream_protocol::endpoint const&
  {
    return _endpoint;
  }
}

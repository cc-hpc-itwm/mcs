// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Name.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/read/declare.hpp>
#include <variant>

namespace mcs::core::transport::implementation::libfabric::provider
{
  template<util::ASIO::is_protocol Protocol>
    struct ConnectionInformation
  {
    util::ASIO::Connectable<Protocol> control;
    libfabric::Name transport;
  };

  using AnyConnectionInformation = std::variant
    < ConnectionInformation<asio::ip::tcp>
    , ConnectionInformation<asio::local::stream_protocol>
    >;

  template<typename Runner, typename... Args>
    auto run (AnyConnectionInformation, Runner&&, Args&&...);
}

namespace fmt
{
  template<mcs::util::ASIO::is_protocol Protocol>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::transport::implementation::libfabric::provider
        ::ConnectionInformation<Protocol>
      )
    ;
}

namespace mcs::util::read
{
  template<util::ASIO::is_protocol Protocol>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::transport::implementation::libfabric::provider
        ::ConnectionInformation<Protocol>
      )
    ;
}

namespace mcs::serialization
{
  template<util::ASIO::is_protocol Protocol>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::transport::implementation::libfabric::provider
        ::ConnectionInformation<Protocol>
      )
    ;
}

#include "detail/ConnectionInformation.ipp"

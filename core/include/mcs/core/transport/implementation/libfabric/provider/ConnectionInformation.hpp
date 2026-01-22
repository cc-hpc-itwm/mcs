// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <fmt/base.h>
#include <mcs/core/transport/implementation/libfabric/libfabric/Name.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
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
    struct formatter<mcs::core::transport::implementation::libfabric::provider::ConnectionInformation<Protocol>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::transport::implementation::libfabric::provider::ConnectionInformation<Protocol> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  }
  ;
}

namespace mcs::util::read
{
  template<util::ASIO::is_protocol Protocol>
    struct Read<core::transport::implementation::libfabric::provider
      ::ConnectionInformation<Protocol>>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> core::transport::implementation::libfabric::provider
          ::ConnectionInformation<Protocol>
        ;
  };
}

namespace mcs::serialization
{
  template<util::ASIO::is_protocol Protocol>
    struct Implementation<core::transport::implementation::libfabric::provider::ConnectionInformation<Protocol>>
  {
    using Type = core::transport::implementation::libfabric::provider::ConnectionInformation<Protocol>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/ConnectionInformation.ipp"

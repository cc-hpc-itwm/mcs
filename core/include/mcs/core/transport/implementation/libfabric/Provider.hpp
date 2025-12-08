// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/Storages.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/transport/implementation/libfabric/Commands.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Interface.hpp>
#include <mcs/core/transport/implementation/libfabric/provider/ConnectionInformation.hpp>
#include <mcs/core/transport/implementation/libfabric/provider/Handler.hpp>
#include <mcs/core/transport/implementation/libfabric/transporter/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::core::transport::implementation::libfabric
{
  template< util::ASIO::is_protocol Protocol
          , transporter::is_provider Transporter
          , typename StorageImplementations
          > struct Provider;

  template< util::ASIO::is_protocol Protocol
          , transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    struct Provider< Protocol
                   , Transporter
                   , util::type::List<StorageImplementations...>
                   >
  {
    template<typename Executor>
      explicit Provider
        ( Executor&
        , typename Protocol::endpoint
        , libfabric::Interface
        , util::not_null
            <Storages<util::type::List<StorageImplementations...>>>
        );

    auto connection_information
      (
      ) const -> provider::ConnectionInformation<Protocol>
      ;

  private:
    using Dispatcher = Commands::template wrap
      < rpc::Dispatcher
      , provider::Handler<Transporter, StorageImplementations...>
      >;

    Transporter _transporter;

    rpc::Provider
      < Protocol
      , Dispatcher
      , util::not_null<Transporter>
      , util::not_null<Storages<util::type::List<StorageImplementations...>>>
      > _controller;
  };
}

#include "detail/Provider.ipp"

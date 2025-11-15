// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/Storages.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/transport/implementation/ASIO/Commands.hpp>
#include <mcs/core/transport/implementation/ASIO/provider/Handler.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::core::transport::implementation::ASIO
{
  template< util::ASIO::is_protocol Protocol
          , typename StorageImplementations
          > struct Provider;

  template< util::ASIO::is_protocol Protocol
          , storage::is_implementation... StorageImplementations
          >
    struct Provider< Protocol
                   , util::type::List<StorageImplementations...>
                   >
  {
    template<typename Executor>
      explicit Provider
         ( Executor&
         , typename Protocol::endpoint
         , util::not_null<Storages<util::type::List<StorageImplementations...>>>
         );

    auto connection_information() const -> util::ASIO::Connectable<Protocol>;

  private:
    using Dispatcher = Commands::template wrap
      < rpc::Dispatcher
      , provider::Handler<StorageImplementations...>
      >;

    rpc::Provider
      < Protocol
      , Dispatcher
      , util::not_null<Storages<util::type::List<StorageImplementations...>>>
      > _provider;
  };

  template< rpc::is_protocol Protocol
          , typename Executor
          , storage::is_implementation... StorageImplementations
          >
    [[nodiscard]] auto make_provider
      ( Executor&
      , typename Protocol::endpoint
      , util::not_null<Storages<util::type::List<StorageImplementations...>>>
      )
    ;
}

#include "detail/Provider.ipp"

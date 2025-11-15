// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <csignal>
#include <mcs/core/Storages.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/share_service/Commands.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <mcs/share_service/provider/Handler.hpp>
#include <mcs/util/ASIO/Connectable.hpp>

namespace mcs::share_service
{
  // Provider of the share_service.
  //
  // EXAMPLE:
  //
  //   // create provider
  //   auto provider { mcs::share_service::Provider<Protocol>
  //                     {io_context, endpoint}
  //                 };
  //
  //   // publish endpoint to be used by clients to connect
  //   fmt::print ( "{}\n"
  //              , mcs::util::ASIO::make_connectable (provider.local_endpoint())
  //              );
  //
  //   // run until io_context terminates
  //   io_context.join();
  //
  template<rpc::is_protocol Protocol>
    struct Provider
  {
    template<typename Executor>
      Provider ( Executor&
               , typename Protocol::endpoint
               );

    auto local_endpoint() const -> typename Protocol::endpoint;

  private:
    core::Storages<SupportedStorageImplementations> _storages;
    rpc::Provider< Protocol
                 , Commands::template wrap
                     < rpc::Dispatcher
                     , provider::Handler<SupportedStorageImplementations>
                     >
                 , core::Storages<SupportedStorageImplementations>&
                 > _provider;
  };
}

#include "detail/Provider.ipp"

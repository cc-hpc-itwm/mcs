// Copyright (C) 2023-2026 Fraunhofer ITWM
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
#include <mcs/util/not_null.hpp>

namespace mcs::share_service
{
  // Provider of the share_service.
  //
  // Storage is borrowed and must outlive the executor and its work.
  // Declare storage before io_context, and stop/join runner threads
  // before destroying the context.
  //
  // EXAMPLE:
  //
  //   auto storages {mcs::share_service::Provider<Protocol>::Storages{}};
  //   auto io_context {mcs::rpc::ScopedRunningIOContext {threads}};
  //
  //   // create provider
  //   auto provider { mcs::share_service::Provider<Protocol>
  //                     { io_context
  //                     , endpoint
  //                     , std::addressof (storages)
  //                     }
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
    using Storages = core::Storages<SupportedStorageImplementations>;

    template<typename Executor>
      Provider ( Executor&
               , typename Protocol::endpoint
               , util::not_null<Storages>
               );

    auto local_endpoint() const -> typename Protocol::endpoint;

  private:
    rpc::Provider< Protocol
                 , Commands::template wrap
                     < rpc::Dispatcher
                     , provider::Handler<SupportedStorageImplementations>
                     >
                 , util::not_null<Storages>
                 > _provider;
  };
}

#include "detail/Provider.ipp"

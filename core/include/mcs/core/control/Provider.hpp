// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/Storages.hpp>
#include <mcs/core/control/Commands.hpp>
#include <mcs/core/control/provider/Handler.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::core::control
{
  template< rpc::is_protocol Protocol
          , typename StorageImplementations
          > struct Provider;

  template< rpc::is_protocol Protocol
          , storage::is_implementation... StorageImplementations
          >
    struct Provider< Protocol
                   , util::type::List<StorageImplementations...>
                   >
  {
    template<typename Executor>
      Provider
        ( Executor&
        , typename Protocol::endpoint
        , util::not_null<Storages<util::type::List<StorageImplementations...>>>
        );

    [[nodiscard]] auto local_endpoint() const -> typename Protocol::endpoint;

  private:
    using Dispatcher = typename provider::Commands<StorageImplementations...>
      ::template wrap< rpc::Dispatcher
                     , provider::Handler<StorageImplementations...>
                     >
      ;
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

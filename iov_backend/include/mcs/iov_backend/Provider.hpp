// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/iov_backend/Commands.hpp>
#include <mcs/iov_backend/provider/Handler.hpp>
#include <mcs/iov_backend/provider/State.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/util/not_null.hpp>

namespace mcs::iov_backend
{
  template<rpc::is_protocol Protocol, typename ExecutorStoragesClients>
    struct Provider
  {
    template<typename Executor>
      Provider ( typename Protocol::endpoint
               , Executor&
               , ExecutorStoragesClients&
               , util::not_null<provider::StoragesClients>
               , provider::State
               );

    auto local_endpoint() const -> typename Protocol::endpoint;

  private:
    provider::State _state;
    rpc::Provider< Protocol
                 , Commands::template wrap
                     < rpc::Dispatcher
                     , provider::Handler<ExecutorStoragesClients>
                     >
                 , ExecutorStoragesClients&
                 , util::not_null<provider::StoragesClients>
                 , provider::State&
                 > _provider
      ;
  };
}

#include "detail/Provider.ipp"

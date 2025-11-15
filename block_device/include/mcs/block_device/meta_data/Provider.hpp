// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/block_device/meta_data/Blocks.hpp>
#include <mcs/block_device/meta_data/Commands.hpp>
#include <mcs/block_device/meta_data/provider/Handler.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/util/not_null.hpp>

namespace mcs::block_device::meta_data
{
  template<rpc::is_protocol Protocol>
    struct Provider
  {
    template<typename Executor>
      Provider ( typename Protocol::endpoint
               , Executor&
               , util::not_null<Blocks>
               );

    auto local_endpoint() const -> typename Protocol::endpoint;

  private:
    rpc::Provider< Protocol
                 , Commands::template wrap
                     < rpc::Dispatcher
                     , provider::Handler
                     >
                 , util::not_null<Blocks>
                 > _provider;
  };
}

#include "detail/Provider.ipp"

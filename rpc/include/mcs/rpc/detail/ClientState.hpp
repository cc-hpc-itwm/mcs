// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/Concepts.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <memory>

namespace mcs::rpc::detail
{
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    struct ClientState
  {
    explicit ClientState
      ( std::shared_ptr<typename Protocol::socket>
      , std::shared_ptr<AccessPolicy>
      ) noexcept;

    template<typename Executor>
      explicit ClientState
        ( Executor&
        , typename Protocol::endpoint
        , std::shared_ptr<AccessPolicy>
        );

    template<typename Executor>
      explicit ClientState
        ( Executor&
        , util::ASIO::Connectable<Protocol>
        , std::shared_ptr<AccessPolicy>
        );

    std::shared_ptr<typename Protocol::socket> socket;
    std::shared_ptr<AccessPolicy> access_policy;
  };
}

#include "detail/ClientState.ipp"

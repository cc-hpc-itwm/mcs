// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <future>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/detail/client/Connection.hpp>
#include <memory>

namespace mcs::rpc::detail
{
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command Command
          , is_command... Commands
          , typename CommandHolder
          >
    requires (is_one_of_the_commands<Command, Commands...>)
    auto remote_call
      ( std::shared_ptr<client::Connection<Protocol, typename AccessPolicy::admission>>
      , std::shared_ptr<AccessPolicy>
      , CommandHolder
      ) -> std::future<typename Command::Response>
    ;
}

#include "detail/remote_call.ipp"

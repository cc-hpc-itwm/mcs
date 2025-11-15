// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/detail/ClientState.hpp>
#include <type_traits>

namespace mcs::rpc::detail
{
  template< is_access_policy_observer Observer
          , is_protocol Protocol
          , is_access_policy AccessPolicy
          , typename... ObserverArgs
          , is_command... Commands
          >
    requires (std::is_constructible_v<Observer, ObserverArgs...>)
    constexpr auto observe
      ( ClientState<Protocol, AccessPolicy, Commands...>
      , ObserverArgs&&...
      ) noexcept (std::is_nothrow_constructible_v<Observer, ObserverArgs...>)
    ;
}

#include "detail/observe.ipp"

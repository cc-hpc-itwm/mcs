// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/access_policy/Observed.hpp>
#include <mcs/rpc/detail/ClientState.hpp>
#include <memory>
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
      ( ClientState<Protocol, AccessPolicy, Commands...> state
      , ObserverArgs&&... observer_args
      ) noexcept (std::is_nothrow_constructible_v<Observer, ObserverArgs...>)
  {
    using ObservedAccessPolicy
      = typename access_policy::Observed<AccessPolicy, Observer>::Type
      ;

    static_assert (is_access_policy<ObservedAccessPolicy>);

    return ClientState<Protocol, ObservedAccessPolicy, Commands...>
     { state.socket
     , std::make_shared<ObservedAccessPolicy>
         ( state.access_policy
         , std::forward<ObserverArgs> (observer_args)...
         )
     };
  }
}

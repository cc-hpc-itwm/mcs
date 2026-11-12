// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/nonstd/scope.hpp>
#include <utility>

namespace mcs::rpc::access_policy
{
  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    template<typename... ObserverArgs>
      requires (std::is_constructible_v<Observer, ObserverArgs...>)
      constexpr Observed<AccessPolicy, Observer>::Observed
        ( std::shared_ptr<AccessPolicy> policy
        , ObserverArgs&&... observer_args
        ) noexcept (std::is_nothrow_constructible_v<Observer, ObserverArgs...>)
          : _policy {std::move (policy)}
          , _observer {std::forward<ObserverArgs> (observer_args)...}
  {}

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    auto Observed<AccessPolicy, Observer>::call_started() noexcept -> void
  {
    auto const notify
      { nonstd::make_scope_exit_that_dies_on_exception
          ( "rpc::Observed::call_started"
          , [&]() noexcept (noexcept (_observer.call_started()))
            {
              _observer.call_started();
            }
          )
      };

    if constexpr (has_call_started<AccessPolicy>)
    {
      _policy->call_started();
    }
  }

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    auto Observed<AccessPolicy, Observer>::call_completed() noexcept -> void
  {
    auto const notify
      { nonstd::make_scope_exit_that_dies_on_exception
          ( "rpc::Observed::call_completed"
          , [&]() noexcept (noexcept (_observer.call_completed()))
            {
              _observer.call_completed();
            }
          )
      };

    if constexpr (has_call_completed<AccessPolicy>)
    {
      _policy->call_completed();
    }
  }

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    auto Observed<AccessPolicy, Observer>::error_execution() noexcept -> void
  {
    auto const notify
      { nonstd::make_scope_exit_that_dies_on_exception
          ( "rpc::Observed::error_execution"
          , [&]() noexcept (noexcept (_observer.error_execution()))
            {
              _observer.error_execution();
            }
          )
      };

    if constexpr (has_error_execution<AccessPolicy>)
    {
      _policy->error_execution();
    }
  }
}

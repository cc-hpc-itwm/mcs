// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/nonstd/scope.hpp>
#include <utility>

namespace mcs::rpc::access_policy
{
  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    template<typename... ObserverArgs>
      requires (std::is_constructible_v<Observer, ObserverArgs...>)
      constexpr ObservedSimple<AccessPolicy, Observer>::ObservedSimple
        ( std::shared_ptr<AccessPolicy> policy
        , ObserverArgs&&... observer_args
        ) noexcept (std::is_nothrow_constructible_v<Observer, ObserverArgs...>)
          : _policy {policy}
          , _observer {std::forward<ObserverArgs> (observer_args)...}
  {}

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    auto ObservedSimple<AccessPolicy, Observer>::start_call
      ( detail::Completion completion
      ) noexcept -> detail::CallID
  {
    auto call_id {_policy->start_call (std::move (completion))};
    _observer.call_started();
    return call_id;
  }

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    auto ObservedSimple<AccessPolicy, Observer>::completion
      ( detail::CallID call_id
      ) -> detail::Completion
  {
    auto const notify
     { nonstd::make_scope_exit_that_dies_on_exception
       ( "rpc::Observed::completion"
       , [&]
         {
           _observer.call_completed();
         }
       )
     };

    return _policy->completion (call_id);
  }

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    auto ObservedSimple<AccessPolicy, Observer>::error
      ( std::exception_ptr rpc_error
      ) noexcept -> void
  {
    auto const notify
      { nonstd::make_scope_exit_that_dies_on_exception
          ( "rpc::Observed::error"
          , [&]
            {
              _observer.error_execution();
            }
          )
      };

    return _policy->error (rpc_error);
  }

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    auto ObservedSentReadLock<AccessPolicy, Observer>::sent() noexcept -> void
  {
    if constexpr (needs_sent_notification<Observer>)
    {
      auto const notify
        { nonstd::make_scope_exit_that_dies_on_exception
            ( "rpc::Obeserbed::sent"
            , [&]
              {
                Base::_observer.sent();
              }
            )
        };

      return Base::_policy->sent();
    }
    else
    {
      return Base::_policy->sent();
    }
  }

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    auto ObservedSentReadLock<AccessPolicy, Observer>::read_lock
      (
      ) noexcept -> decltype (Base::_policy->read_lock())
  {
    if constexpr (needs_read_lock<Observer>)
    {
      auto const notify
        { nonstd::make_scope_exit_that_dies_on_exception
            ( "rpc::Observed::read_lock"
            , [&]
              {
                Base::_observer.read_locked();
              }
            )
        };

      return Base::_policy->read_lock();
    }
    else
    {
      return Base::_policy->read_lock();
    }
  }
}

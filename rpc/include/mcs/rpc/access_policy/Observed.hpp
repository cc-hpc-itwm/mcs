// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/detail/CallID.hpp>
#include <mcs/rpc/detail/Completion.hpp>
#include <memory>
#include <type_traits>

namespace mcs::rpc::access_policy
{
  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    struct ObservedSimple
  {
    template<typename... ObserverArgs>
      requires (std::is_constructible_v<Observer, ObserverArgs...>)
      constexpr explicit ObservedSimple
        ( std::shared_ptr<AccessPolicy>
        , ObserverArgs&&...
        ) noexcept (std::is_nothrow_constructible_v<Observer, ObserverArgs...>)
      ;

    [[nodiscard]] auto start_call (detail::Completion) noexcept -> detail::CallID;
    [[nodiscard]] auto completion (detail::CallID) -> detail::Completion;

    auto error (std::exception_ptr) noexcept -> void;

  protected:
    std::shared_ptr<AccessPolicy> _policy;
    Observer _observer;
  };

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    struct ObservedSentReadLock : public ObservedSimple<AccessPolicy, Observer>
  {
    using Base = ObservedSimple<AccessPolicy, Observer>;
    using Base::Base;

    auto sent() noexcept -> void;
    [[nodiscard]] auto read_lock() noexcept
      -> decltype (Base::_policy->read_lock());
  };

  template< is_access_policy AccessPolicy
          , is_access_policy_observer Observer
          , bool // needs_sent_notification<AccessPolicy>
          , bool // needs_read_lock<AccessPolicy>
          >
    struct ObservedT;

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    struct ObservedT<AccessPolicy, Observer, false, false>
  {
    using Type = ObservedSimple<AccessPolicy, Observer>;
  };
  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    struct ObservedT<AccessPolicy, Observer, true, true>
  {
    using Type = ObservedSentReadLock<AccessPolicy, Observer>;
  };

  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    struct Observed : public ObservedT< AccessPolicy
                                      , Observer
                                      , needs_sent_notification<AccessPolicy>
                                      , needs_read_lock<AccessPolicy>
                                      >
  {};
}

#include "detail/Observed.ipp"

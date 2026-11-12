// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/Concepts.hpp>
#include <memory>
#include <type_traits>

namespace mcs::rpc::access_policy
{
  template<is_access_policy AccessPolicy, is_access_policy_observer Observer>
    struct Observed
  {
    using admission = typename AccessPolicy::admission;

    template<typename... ObserverArgs>
      requires (std::is_constructible_v<Observer, ObserverArgs...>)
      constexpr explicit Observed
        ( std::shared_ptr<AccessPolicy>
        , ObserverArgs&&...
        ) noexcept (std::is_nothrow_constructible_v<Observer, ObserverArgs...>)
      ;

    auto call_started() noexcept -> void;
    auto call_completed() noexcept -> void;
    auto error_execution() noexcept -> void;

  private:
    std::shared_ptr<AccessPolicy> _policy;
    Observer _observer;
  };
}

#include "detail/Observed.ipp"

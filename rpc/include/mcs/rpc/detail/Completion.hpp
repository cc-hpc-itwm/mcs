// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <functional>
#include <future>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/detail/Buffer.hpp>
#include <memory>

namespace mcs::rpc::detail
{
  struct Completion
  {
    template<typename T, is_access_policy AccessPolicy>
      constexpr explicit Completion (std::promise<T>, std::shared_ptr<AccessPolicy>);

    auto operator() (std::exception_ptr) -> void;
    auto operator() (Buffer) -> void;

    Completion (Completion const&) = delete;
    Completion (Completion&&) noexcept = default;
    auto operator= (Completion const&) -> Completion& = delete;
    auto operator= (Completion&&) noexcept -> Completion& = default;
    ~Completion() noexcept = default;

  private:
    struct Initialize{};

    template<typename T, is_access_policy AccessPolicy>
      requires (  !has_call_completed<AccessPolicy>
               && !has_error_execution<AccessPolicy>
               )
      constexpr Completion
        ( Initialize
        , std::promise<T>
        , std::shared_ptr<AccessPolicy>
        )
      ;

    template<typename T, is_access_policy AccessPolicy>
      requires (  has_call_completed<AccessPolicy>
               || has_error_execution<AccessPolicy>
               )
      constexpr Completion
        ( Initialize
        , std::promise<T>
        , std::shared_ptr<AccessPolicy>
        )
      ;

    // The completion owns the promise and, only when notifications
    // are requested, the policy.
    //
    std::function<void (std::exception_ptr, Buffer)> _complete;
  };
}

#include "detail/Completion.ipp"

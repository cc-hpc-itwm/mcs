// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <mcs/rpc/detail/CallID.hpp>
#include <mcs/rpc/detail/Completion.hpp>
#include <memory>

namespace mcs::rpc::access_policy
{
  struct Exclusive
  {
    [[nodiscard]] auto start_call (detail::Completion) noexcept -> detail::CallID;
    [[nodiscard]] auto completion (detail::CallID) -> detail::Completion;

    auto error (std::exception_ptr) noexcept -> void;

  private:
    detail::CallID _call_id{};

    [[nodiscard]] auto completion() -> detail::Completion;

    std::unique_ptr<detail::Completion> _completion {nullptr};
  };
}

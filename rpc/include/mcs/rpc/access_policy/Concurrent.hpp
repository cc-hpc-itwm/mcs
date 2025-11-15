// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <map>
#include <mcs/rpc/detail/CallID.hpp>
#include <mcs/rpc/detail/Completion.hpp>
#include <memory>
#include <mutex>

namespace mcs::rpc::access_policy
{
  struct Concurrent
  {
    [[nodiscard]] auto start_call (detail::Completion) noexcept -> detail::CallID;
    [[nodiscard]] auto completion (detail::CallID) -> detail::Completion;

    auto error (std::exception_ptr) noexcept -> void;

    auto sent() noexcept -> void;
    [[nodiscard]] auto read_lock() noexcept
      -> std::unique_ptr<std::lock_guard<std::mutex>>;

  private:
    std::mutex _guard_send;
    std::unique_ptr<std::lock_guard<decltype (_guard_send)>> _lock_send;
    detail::CallID _call_id{};

    std::mutex _guard_completions;
    //! \todo unordered? ring_buffer?
    std::map<detail::CallID, detail::Completion> _completions;

    std::mutex _guard_read;
  };
}

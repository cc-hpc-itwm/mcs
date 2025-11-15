// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <condition_variable>
#include <mcs/rpc/multi_client/ParallelCallsLimit.hpp>
#include <mcs/rpc/multi_client/detail/CallID.hpp>
#include <mutex>
#include <unordered_set>

namespace mcs::rpc::multi_client::detail
{
  // Counters for a single wave. Please note that a "wave" might
  // outlive a "call", where a "call" has ended when
  // "call_and_collect!" has returned.
  //
  struct Counters
  {
  public:
    explicit Counters (multi_client::ParallelCallsLimit) noexcept;

    // Returns: The (possibly empty set of) ids of all calls that have
    // completed since the last call to wait_for_slot().
    //
    using CallIDs = std::unordered_set<CallID>;
    [[nodiscard]] auto wait_for_slot() -> CallIDs;

    // Returns: The (non-empty set of) ids of all calls that have
    // completed since the last call to wait_for_slot() or
    // calls_completed(). Waits for at least one call to finish.
    //
    // \note If calls_completed() is called after all calls have been
    // completed, then it will wait forever.
    [[nodiscard]] auto calls_completed() -> CallIDs;

  private:
    std::mutex _guard;
    std::condition_variable _completed;
    multi_client::ParallelCallsLimit _parallel_calls_limit;
    unsigned int _call_started {0u};
    unsigned int _call_completed {0u};
    unsigned int _error_execution {0u};
    CallIDs _calls_started;
    CallIDs _calls_completed;

    using Guarded = std::lock_guard<decltype (_guard)>;
    auto start (Guarded const&, CallID) -> void;
    auto complete (Guarded const&, CallID) -> void;

    using Locked = std::unique_lock<decltype (_guard)>;
    constexpr auto finished (Locked const&) const noexcept -> unsigned int;
    constexpr auto has_slot_available (Locked const&) const -> bool;

    friend struct ClientObserver;
    auto call_started (CallID) -> void;
    auto call_completed (CallID) -> void;
    auto error_execution (CallID) -> void;
  };
}

#include "detail/Counters.ipp"

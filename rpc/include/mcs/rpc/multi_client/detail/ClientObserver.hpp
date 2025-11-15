// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/multi_client/detail/CallID.hpp>
#include <mcs/rpc/multi_client/detail/Counters.hpp>

namespace mcs::rpc::multi_client::detail
{
  // specific for one wave, stored in the access policy observer, may
  // outlive the call
  //
  struct ClientObserver
  {
    explicit ClientObserver (CallID, Counters&) noexcept;

    auto call_started() noexcept -> void;
    auto call_completed() -> void;
    auto error_execution() -> void;

  private:
    CallID _call_id;
    Counters& _counters;
  };
}

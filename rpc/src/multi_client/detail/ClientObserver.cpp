// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/multi_client/detail/ClientObserver.hpp>

namespace mcs::rpc::multi_client::detail
{
  ClientObserver::ClientObserver
    ( CallID call_id
    , Counters& counters
    ) noexcept
      : _call_id {call_id}
      , _counters {counters}
  {}

  auto ClientObserver::call_started() noexcept -> void
  {
    return _counters.call_started (_call_id);
  }

  auto ClientObserver::call_completed() -> void
  {
    return _counters.call_completed (_call_id);
  }

  auto ClientObserver::error_execution() -> void
  {
    return _counters.error_execution (_call_id);
  }
}

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/overloaded.hpp>

namespace mcs::rpc::multi_client::detail
{
  constexpr auto Counters::finished
    ( Locked const&
    ) const noexcept -> unsigned int
  {
    // the remote_call guarantees to call either of them
    return _call_completed + _error_execution;
  }

  constexpr auto Counters::has_slot_available
    ( Locked const& lock
    ) const -> bool
  {
    return std::visit
      ( util::overloaded
        ( [] (multi_client::ParallelCalls::Unlimited) noexcept
          {
            return true;
          }
        , [&] (multi_client::ParallelCalls::AtMost max) noexcept
          {
            return _call_started < finished (lock) + max.value;
          }
        )
      , _parallel_calls_limit
      );
  }
}

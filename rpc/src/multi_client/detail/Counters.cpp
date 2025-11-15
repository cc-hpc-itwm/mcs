// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/rpc/multi_client/detail/Counters.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::rpc::multi_client::detail
{
  Counters::Counters
    ( multi_client::ParallelCallsLimit parallel_calls_limit
    ) noexcept
      : _parallel_calls_limit {parallel_calls_limit}
  {}

  auto Counters::call_started (CallID call_id) -> void
  try
  {
    auto const lock {std::lock_guard {_guard}};

    ++_call_started;
    start (lock, call_id);
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
          { fmt::format ( "mcs::rpc::multi_client::call_started ({})"
                        , call_id
                        )
          }
      );
  }

  auto Counters::call_completed (CallID call_id) -> void
  try
  {
    auto const lock {std::lock_guard {_guard}};

    ++_call_completed;
    complete (lock, call_id);
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
          { fmt::format ( "mcs::rpc::multi_client::call_completed ({})"
                        , call_id
                        )
          }
      );
  }

  auto Counters::error_execution (CallID call_id) -> void
  try
  {
    auto const lock {std::lock_guard {_guard}};

    ++_error_execution;
    complete (lock, call_id);
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
          { fmt::format ( "mcs::rpc::multi_client::error_execution ({})"
                        , call_id
                        )
          }
      );
  }

  auto Counters::wait_for_slot() -> CallIDs
  {
    auto lock {std::unique_lock {_guard}};

    _completed.wait (lock, [&] { return has_slot_available (lock); });

    return std::exchange (_calls_completed, decltype (_calls_completed){});
  }

  auto Counters::calls_completed() -> CallIDs
  {
    auto lock {std::unique_lock {_guard}};

    _completed.wait (lock, [&] { return !_calls_completed.empty(); });

    return std::exchange (_calls_completed, decltype (_calls_completed){});
  }

  auto Counters::start (Guarded const&, CallID call_id) -> void
  {
    if (!_calls_started.emplace (call_id).second)
    {
      throw std::logic_error {"Duplicate call id."};
    }
  }

  auto Counters::complete (Guarded const&, CallID call_id) -> void
  {
    if (_calls_started.erase (call_id) != 1)
    {
      throw std::logic_error {"Unknown call_id"};
    }

    if (!_calls_completed.emplace (call_id).second)
    {
      throw std::logic_error {"Duplicate call id."};
    }

    _completed.notify_one();
  }
}

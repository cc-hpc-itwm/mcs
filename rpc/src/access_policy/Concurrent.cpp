// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <iterator>
#include <mcs/rpc/access_policy/Concurrent.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::rpc::access_policy
{
  auto Concurrent::start_call
    ( detail::Completion completion
    ) noexcept -> detail::CallID
  {
    _lock_send
      = std::make_unique<std::lock_guard<decltype (_guard_send)>> (_guard_send)
      ;

    auto const lock {std::lock_guard {_guard_completions}};

    _completions.emplace (_call_id, std::move (completion));

    return _call_id++;
  }

  auto Concurrent::completion (detail::CallID call_id) -> detail::Completion
  {
    auto const lock {std::lock_guard {_guard_completions}};

    auto pos {_completions.find (call_id)};

    if (pos == std::end (_completions))
    {
      throw std::logic_error {"Unknown call_id."};
    }

    auto completion {std::move (pos->second)};

    _completions.erase (pos);

    return completion;
  }

  auto Concurrent::error (std::exception_ptr rpc_error) noexcept -> void
  {
    auto const lock {std::lock_guard {_guard_completions}};

    for (auto& completion : _completions)
    {
      completion.second (rpc_error);
    }

    _completions.clear();
  }

  auto Concurrent::sent() noexcept -> void
  {
    // allow the next command to be send
    _lock_send.reset();
  }

  auto Concurrent::read_lock
    (
    ) noexcept -> std::unique_ptr<std::lock_guard<std::mutex>>
  {
    return std::make_unique<std::lock_guard<std::mutex>> (_guard_read);
  }
}

// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/rpc/access_policy/Sequential.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::rpc::access_policy
{
  auto Sequential::start_call
    ( detail::Completion completion
    ) noexcept -> detail::CallID
  {
    _lock_send
      = std::make_unique<std::lock_guard<decltype (_guard_send)>> (_guard_send)
      ;

    _completion = std::make_unique<detail::Completion> (std::move (completion));

    return _call_id;
  }

  auto Sequential::completion (detail::CallID call_id) -> detail::Completion
  {
    if (call_id != _call_id)
    {
      throw std::logic_error {fmt::format ("Expected call_id {}", _call_id)};
    }

    return completion();
  }

  auto Sequential::error (std::exception_ptr rpc_error) noexcept -> void
  {
    completion() (rpc_error);
  }

  auto Sequential::completion() -> detail::Completion
  {
    if (!_lock_send || !_completion)
    {
      throw std::logic_error {"No completion."};
    }

    ++_call_id;

    auto completion {std::move (*_completion)}; _completion.reset();

    _lock_send.reset();

    return completion;
  }
}

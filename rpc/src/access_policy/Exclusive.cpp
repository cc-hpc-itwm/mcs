// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::rpc::access_policy
{
  auto Exclusive::start_call
    ( detail::Completion completion
    ) noexcept -> detail::CallID
  {
    _completion = std::make_unique<detail::Completion> (std::move (completion));

    return _call_id;
  }

  auto Exclusive::completion() -> detail::Completion
  {
    if (!_completion)
    {
      throw std::logic_error {"No completion."};
    }

    ++_call_id;

    auto completion {std::move (*_completion)}; _completion.reset();

    return completion;
  }

  auto Exclusive::error (std::exception_ptr rpc_error) noexcept -> void
  {
    completion() (rpc_error);
  }

  auto Exclusive::completion (detail::CallID call_id) -> detail::Completion
  {
    if (call_id != _call_id)
    {
      throw std::logic_error
        {fmt::format ("Expected call_id {}, got {}", _call_id, call_id)};
    }

    return completion();
  }
}

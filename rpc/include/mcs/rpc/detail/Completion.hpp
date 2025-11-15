// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <functional>
#include <future>
#include <mcs/rpc/detail/Buffer.hpp>

namespace mcs::rpc::detail
{
  struct Completion
  {
    template<typename T>
      constexpr explicit Completion (std::promise<T>);

    auto operator() (std::exception_ptr) -> void;
    auto operator() (Buffer) -> void;

    Completion (Completion const&) = delete;
    Completion (Completion&&) noexcept = default;
    auto operator= (Completion const&) -> Completion& = delete;
    auto operator= (Completion&&) noexcept -> Completion& = default;
    ~Completion() noexcept = default;

  private:
    std::function<void (std::exception_ptr, Buffer)> _complete;
  };
}

#include "detail/Completion.ipp"

// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <iov/iov.hpp>
#include <span>

namespace mcs::iov_backend::detail
{
  struct SpanDeleter
  {
    explicit constexpr SpanDeleter (void*, iov::BackendOps&) noexcept;
    auto operator() (std::span<std::byte>*) const noexcept -> void;

  private:
    void* _backend_state;
    iov::BackendOps& _backend_ops;
  };
}

#include "detail/SpanDeleter.ipp"

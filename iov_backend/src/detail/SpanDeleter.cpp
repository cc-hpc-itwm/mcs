// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/iov_backend/detail/SpanDeleter.hpp>
#include <memory>

namespace mcs::iov_backend::detail
{
  auto SpanDeleter::operator()
    ( std::span<std::byte>* span
    ) const noexcept -> void
  {
    _backend_ops.iov_backend_free (_backend_state, *span);

    std::default_delete<std::span<std::byte>>{} (span);
  }
}

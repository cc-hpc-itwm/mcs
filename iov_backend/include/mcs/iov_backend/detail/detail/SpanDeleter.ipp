// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::iov_backend::detail
{
  constexpr SpanDeleter::SpanDeleter
    ( void* backend_state
    , iov::BackendOps& backend_ops
    ) noexcept
      : _backend_state {backend_state}
      , _backend_ops {backend_ops}
  {}
}

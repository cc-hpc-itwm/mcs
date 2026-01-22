// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/multi_client/detail/CallID.hpp>

namespace std
{
  auto hash<mcs::rpc::multi_client::detail::CallID>::operator()
    ( mcs::rpc::multi_client::detail::CallID x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x._value);
  }
}

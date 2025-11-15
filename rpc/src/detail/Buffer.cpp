// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/detail/Buffer.hpp>

namespace mcs::rpc::detail
{
  auto Buffer::modifiable() const noexcept -> asio::mutable_buffer
  {
    auto bytes {data<std::byte>()};

    return {bytes.data(), bytes.size()};
  }
}

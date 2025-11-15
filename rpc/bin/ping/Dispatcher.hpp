// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/serialization/declare.hpp>
#include <vector>

namespace mcs::rpc::ping
{
  struct Ping
  {
    explicit Ping (std::vector<std::byte>);
    explicit Ping (unsigned int message_size);

    std::vector<std::byte> data;

    using Response = std::vector<std::byte>;
  };

  struct Handler
  {
    auto operator() (Ping ping) -> Ping::Response;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION (rpc::ping::Ping);
}

namespace mcs::rpc::ping
{
  using Dispatcher = rpc::Dispatcher<Handler, Ping>;
}

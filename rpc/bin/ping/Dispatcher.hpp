// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/serialization/Concepts.hpp>
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
    struct Implementation<rpc::ping::Ping>
  {
    using Type = rpc::ping::Ping;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::rpc::ping
{
  using Dispatcher = rpc::Dispatcher<Handler, Ping>;
}

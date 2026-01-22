// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "Dispatcher.hpp"
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::rpc::ping
{
  Ping::Ping (std::vector<std::byte> data_)
    : data {data_}
  {}
  Ping::Ping (unsigned int message_size)
    : data {message_size}
  {}

  auto Handler::operator() (Ping ping) -> Ping::Response
  {
    return ping.data;
  }
}

namespace mcs::serialization
{
  auto Implementation<mcs::rpc::ping::Ping>::output
    ( OArchive& oa
    , mcs::rpc::ping::Ping const& value
    ) -> OArchive&
  {
    save (oa, value.data);

    return oa;
  }
  auto Implementation<mcs::rpc::ping::Ping>::input
    ( IArchive& ia
    ) -> mcs::rpc::ping::Ping
  {
    auto data {load<decltype (mcs::rpc::ping::Ping::data)> (ia)};

    return mcs::rpc::ping::Ping {data};
  }
}

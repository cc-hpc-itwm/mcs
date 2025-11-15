// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "Dispatcher.hpp"
#include <mcs/util/tuplish/define.hpp>
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

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::rpc::ping::Ping
  , data
  );

// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/awaitable.hpp>
#include <mcs/rpc/detail/Buffer.hpp>
#include <tuple>

namespace mcs::rpc::detail
{
  template<typename Header, typename Socket>
    auto receive_buffer_with_header (Socket&)
      -> asio::awaitable<std::tuple<Header, Buffer>>
    ;
}

#include "detail/receive_buffer_with_header.ipp"

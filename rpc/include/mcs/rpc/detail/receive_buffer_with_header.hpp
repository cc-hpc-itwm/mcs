// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/awaitable.hpp>
#include <mcs/rpc/detail/ReceivedFrame.hpp>
#include <memory>

namespace mcs::rpc::detail
{
  template<typename Header, typename Socket>
    auto receive_buffer_with_header (std::shared_ptr<Socket>)
      -> asio::awaitable<ReceivedFrame<Header>>
    ;
}

#include "detail/receive_buffer_with_header.ipp"

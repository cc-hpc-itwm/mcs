// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/rpc/detail/Buffer.hpp>

namespace mcs::rpc::detail
{
  template<typename Header>
    struct ReceivedFrame
  {
    Header header;
    Buffer buffer;
  };
}

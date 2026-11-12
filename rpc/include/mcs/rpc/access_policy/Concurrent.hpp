// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::rpc::access_policy
{
  struct Concurrent
  {
    using admission = Concurrent;
  };
}

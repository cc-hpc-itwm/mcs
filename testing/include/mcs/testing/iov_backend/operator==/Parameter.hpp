// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/iov_backend/Parameter.hpp>
#include <mcs/testing/RPC/operator==/ScopedRunningIOContext/NumberOfThreads.hpp>
#include <tuple>

namespace mcs::iov_backend
{
  [[nodiscard]] constexpr auto operator==
    ( Parameter const& lhs
    , Parameter const& rhs
    )
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie
            ( x._provider

            , x._number_of_threads._transport_clients

            , x._indirect_communication._number_of_buffers
            , x._indirect_communication._maximum_transfer_size
            , x._indirect_communication._acquire_buffer_timeout_in_milliseconds

            , x._direct_communication._maximum_transfer_size
            );
        }
      };

    return essence (lhs) == essence (rhs);
  }
}

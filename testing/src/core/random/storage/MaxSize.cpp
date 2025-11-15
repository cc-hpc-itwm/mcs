// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/random/storage/MaxSize.hpp>
#include <mcs/testing/random/value/define.hpp>

namespace mcs::testing::random
{
  auto value<core::storage::MaxSize>::operator()
    (
    ) -> core::storage::MaxSize
  {
    if (_cent() < 33)
    {
      return core::storage::MaxSize::Unlimited{};
    }
    else
    {
      return core::storage::MaxSize::Limit {_memory_size()};
    }
  }
}

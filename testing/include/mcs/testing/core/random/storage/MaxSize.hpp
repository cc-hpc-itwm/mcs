// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/random/value/integral.hpp>

namespace mcs::testing::random
{
  template<> struct value<core::storage::MaxSize>
  {
    auto operator()() -> core::storage::MaxSize;

  private:
    value<int> _cent {0, 100};
    value<core::memory::Size> _memory_size;
  };
}

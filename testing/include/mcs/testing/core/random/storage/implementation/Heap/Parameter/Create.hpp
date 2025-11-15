// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/random/storage/MaxSize.hpp>

namespace mcs::testing::random
{
  template<>
    struct value<core::storage::implementation::Heap::Parameter::Create>
  {
    auto operator()
      (
      ) -> core::storage::implementation::Heap::Parameter::Create
      ;

  private:
    value<core::storage::MaxSize> _max_size;
  };
}

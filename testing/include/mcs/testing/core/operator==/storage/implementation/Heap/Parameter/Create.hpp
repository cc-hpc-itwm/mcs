// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/operator==/storage/MaxSize.hpp>
#include <tuple>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( typename Heap::Parameter::Create const& lhs
    , typename Heap::Parameter::Create const& rhs
    ) -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie (x.max_size);
        }
      };

    return essence (lhs) == essence (rhs);
  }
}

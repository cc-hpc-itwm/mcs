// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Heap.hpp>
#include <tuple>

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    constexpr auto operator==
      ( Heap::Chunk::Description<Access> const& lhs
      , Heap::Chunk::Description<Access> const& rhs
      ) noexcept -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie ( x._begin
                          , x._size
                          , x._range
                          );
        }
      };

    return essence (lhs) == essence (rhs);
  }
}

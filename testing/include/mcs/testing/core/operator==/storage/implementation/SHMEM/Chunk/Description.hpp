// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/core/operator==/storage/implementation/SHMEM/Prefix.hpp>
#include <tuple>

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    constexpr auto operator==
      ( SHMEM::Chunk::Description<Access> const& lhs
      , SHMEM::Chunk::Description<Access> const& rhs
      ) noexcept -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie ( x.prefix
                          , x.segment_id
                          , x.size
                          , x.range
                          );
        }
      };

    return essence (lhs) == essence (rhs);
  }
}

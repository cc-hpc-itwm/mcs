// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
#include <tuple>

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    constexpr auto operator==
      ( Files::Chunk::Description<Access> const& lhs
      , Files::Chunk::Description<Access> const& rhs
      ) noexcept -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie ( x.path
                          , x.file_size
                          , x.range
                          );
        }
      };

    return essence (lhs) == essence (rhs);
  }
}

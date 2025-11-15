// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/event/chunk/Description.hpp>
#include <tuple>

namespace mcs::core::storage::trace::event::chunk
{
  template<is_implementation Storage, core::chunk::is_access Access>
    [[nodiscard]] constexpr auto operator==
      ( Description<Storage, Access> const& lhs
      , Description<Storage, Access> const& rhs
      ) -> bool
  {
    auto const essence
      { [&] (auto const& x)
        {
          return std::tie ( x._parameter_chunk_description
                          , x._segment_id
                          , x._memory_range
                          );
        }
      };

    return std::equal_to{} (essence (lhs), essence (rhs));
  }
}

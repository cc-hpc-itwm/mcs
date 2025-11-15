// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/event/segment/Create.hpp>
#include <tuple>

namespace mcs::core::storage::trace::event::segment
{
  template<is_implementation Storage>
    [[nodiscard]] constexpr auto operator==
      ( Create<Storage> const& lhs
      , Create<Storage> const& rhs
      ) -> bool
  {
    auto const essence
      { [&] (auto const& x)
        {
          return std::tie ( x._parameter_segment_create
                          , x._size
                          );
        }
      };

    return std::equal_to{} (essence (lhs), essence (rhs));
  }
}

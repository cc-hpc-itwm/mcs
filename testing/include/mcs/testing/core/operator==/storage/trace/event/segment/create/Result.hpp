// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/event/segment/create/Result.hpp>
#include <tuple>

namespace mcs::core::storage::trace::event::segment::create
{
  [[nodiscard]] constexpr auto operator==
    ( Result const& lhs
    , Result const& rhs
    ) -> bool
  {
    auto const essence
      { [&] (auto const& x)
        {
          return std::tie (x._segment_id);
        }
      };

    return std::equal_to{} (essence (lhs), essence (rhs));
  }
}

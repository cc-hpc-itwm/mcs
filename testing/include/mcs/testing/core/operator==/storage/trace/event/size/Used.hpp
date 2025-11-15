// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/event/size/Used.hpp>
#include <tuple>

namespace mcs::core::storage::trace::event::size
{
  template<is_implementation Storage>
    [[nodiscard]] constexpr auto operator==
      ( Used<Storage> const& lhs
      , Used<Storage> const& rhs
      ) -> bool
  {
    auto const essence
      { [&] (auto const& x)
        {
          return std::tie (x._parameter_size_used);
        }
      };

    return std::equal_to{} (essence (lhs), essence (rhs));
  }
}

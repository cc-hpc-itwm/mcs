// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/event/size/Max.hpp>
#include <tuple>

namespace mcs::core::storage::trace::event::size
{
  template<is_implementation Storage>
    [[nodiscard]] constexpr auto operator==
      ( Max<Storage> const& lhs
      , Max<Storage> const& rhs
      ) -> bool
  {
    auto const essence
      { [&] (auto const& x)
        {
          return std::tie (x._parameter_size_max);
        }
      };

    return std::equal_to{} (essence (lhs), essence (rhs));
  }
}

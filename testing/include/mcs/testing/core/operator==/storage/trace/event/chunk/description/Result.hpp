// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/event/chunk/description/Result.hpp>
#include <tuple>

namespace mcs::core::storage::trace::event::chunk::description
{
  template<is_implementation Storage, core::chunk::is_access Access>
    [[nodiscard]] constexpr auto operator==
      ( Result<Storage, Access> const& lhs
      , Result<Storage, Access> const& rhs
      ) -> bool
  {
    auto const essence
      { [&] (auto const& x)
        {
          return std::tie (x._chunk_description);
        }
      };

    return std::equal_to{} (essence (lhs), essence (rhs));
  }
}

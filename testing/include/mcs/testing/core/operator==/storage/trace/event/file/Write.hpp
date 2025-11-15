// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/event/file/Write.hpp>
#include <tuple>

namespace mcs::core::storage::trace::event::file
{
  template<is_implementation Storage>
    [[nodiscard]] constexpr auto operator==
      ( Write<Storage> const& lhs
      , Write<Storage> const& rhs
      ) -> bool
  {
    auto const essence
      { [&] (auto const& x)
        {
          return std::tie ( x._parameter_file_write
                          , x._segment_id
                          , x._offset
                          , x._path
                          , x._range
                          );
        }
      };

    return std::equal_to{} (essence (lhs), essence (rhs));
  }
}

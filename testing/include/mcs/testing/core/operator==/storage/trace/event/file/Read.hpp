// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/event/file/Read.hpp>
#include <tuple>

namespace mcs::core::storage::trace::event::file
{
  template<is_implementation Storage>
    [[nodiscard]] constexpr auto operator==
      ( Read<Storage> const& lhs
      , Read<Storage> const& rhs
      ) -> bool
  {
    auto const essence
      { [&] (auto const& x)
        {
          return std::tie ( x._parameter_file_read
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

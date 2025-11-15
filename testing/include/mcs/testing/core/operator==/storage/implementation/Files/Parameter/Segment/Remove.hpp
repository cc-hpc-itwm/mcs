// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
#include <tuple>
#include <type_traits>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( Files::Parameter::Segment::ForceRemoval const&
    , Files::Parameter::Segment::ForceRemoval const&
    ) noexcept -> bool
  {
    static_assert (std::is_empty_v<Files::Parameter::Segment::ForceRemoval>);

    return true;
  }

  constexpr auto operator==
    ( Files::Parameter::Segment::Remove const& lhs
    , Files::Parameter::Segment::Remove const& rhs
    ) noexcept -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie (x.force_removal);
        }
      };

    return essence (lhs) == essence (rhs);
  }
}

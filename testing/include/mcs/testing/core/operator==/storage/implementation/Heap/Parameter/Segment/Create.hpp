// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Heap.hpp>
#include <tuple>
#include <type_traits>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( Heap::Parameter::Segment::MLOCKed const&
    , Heap::Parameter::Segment::MLOCKed const&
    ) noexcept -> bool
  {
    static_assert (std::is_empty_v<Heap::Parameter::Segment::MLOCKed>);

    return true;
  }

  constexpr auto operator==
    ( Heap::Parameter::Segment::Create const& lhs
    , Heap::Parameter::Segment::Create const& rhs
    ) noexcept -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie (x.mlocked);
        }
      };

    return essence (lhs) == essence (rhs);
  }
}

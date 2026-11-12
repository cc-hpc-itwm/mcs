// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Parameter/Persistency.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Parameter/Segment/OnRemove/Keep.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Parameter/Segment/OnRemove/Remove.hpp>
#include <tuple>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( Files::Parameter::Segment::Create const& lhs
    , Files::Parameter::Segment::Create const& rhs
    ) -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie (x.persistency);
        }
      };

    return essence (lhs) == essence (rhs);
  }
}

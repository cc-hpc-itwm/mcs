// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Parameter/Segment/OnRemove/Keep.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Parameter/Segment/OnRemove/Remove.hpp>
#include <mcs/util/overloaded.hpp>
#include <variant>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( typename Files::Parameter::Segment::Persistency const& lhs
    , typename Files::Parameter::Segment::Persistency const& rhs
    ) -> bool
  {
    auto const essence
      { [] (auto const& value)
        {
          using OnRemove = Files::Parameter::Segment::OnRemove;

          return value.visit
            ( util::overloaded
              ( [] ( auto p
                   ) noexcept -> std::variant<OnRemove::Keep, OnRemove::Remove>
                {
                  return p;
                }
              )
            );
        }
      };

    return essence (lhs) == essence (rhs);
  }
}

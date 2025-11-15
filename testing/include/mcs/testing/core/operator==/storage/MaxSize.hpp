// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/util/overloaded.hpp>
#include <variant>

namespace mcs::core::storage
{
  constexpr auto operator==
    ( MaxSize const& lhs
    , MaxSize const& rhs
    ) -> bool
  {
    return std::visit
      ( util::overloaded
        ( [] (MaxSize::Unlimited, MaxSize::Unlimited) noexcept
          {
            return true;
          }
        , [] (MaxSize::Limit, MaxSize::Unlimited) noexcept
          {
            return false;
          }
        , [] (MaxSize::Unlimited, MaxSize::Limit) noexcept
          {
            return false;
          }
        , [] (MaxSize::Limit const& l, MaxSize::Limit const& r) noexcept
          {
            return l.size == r.size;
          }
        )
      , lhs._limit
      , rhs._limit
      );
  }
}

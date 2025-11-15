// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
#include <tuple>
#include <type_traits>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( Files::Parameter::Segment::OnRemove::Keep const&
    , Files::Parameter::Segment::OnRemove::Keep const&
    ) noexcept -> bool
  {
    static_assert (std::is_empty_v<Files::Parameter::Segment::OnRemove::Keep>);

    return true;
  }

  constexpr auto operator==
    ( Files::Parameter::Segment::OnRemove::Remove const&
    , Files::Parameter::Segment::OnRemove::Remove const&
    ) noexcept -> bool
  {
    static_assert (std::is_empty_v<Files::Parameter::Segment::OnRemove::Remove>);

    return true;
  }

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

// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
#include <type_traits>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( Files::Parameter::Segment::OnRemove::Remove const&
    , Files::Parameter::Segment::OnRemove::Remove const&
    ) noexcept -> bool
  {
    static_assert
      ( std::is_empty_v<Files::Parameter::Segment::OnRemove::Remove>
      );

    return true;
  }
}

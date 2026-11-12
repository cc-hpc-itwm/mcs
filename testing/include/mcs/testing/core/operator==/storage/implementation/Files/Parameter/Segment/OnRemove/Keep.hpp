// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
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
}

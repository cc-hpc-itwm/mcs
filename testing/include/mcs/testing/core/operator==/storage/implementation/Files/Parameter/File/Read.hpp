// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
#include <type_traits>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( Files::Parameter::File::Read const&
    , Files::Parameter::File::Read const&
    ) noexcept -> bool
  {
    static_assert (std::is_empty_v<Files::Parameter::File::Read>);

    return true;
  }
}

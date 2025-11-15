// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/trace/event/Destruct.hpp>
#include <type_traits>

namespace mcs::core::storage::trace::event
{
  [[nodiscard]] constexpr auto operator==
    ( Destruct const&
    , Destruct const&
    ) -> bool
  {
    static_assert (std::is_empty_v<Destruct>);

    return true;
  }
}

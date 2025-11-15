// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>

namespace mcs::core::storage::implementation
{
  auto operator==
    ( typename Files::Prefix const&
    , typename Files::Prefix const&
    ) noexcept -> bool;
}

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/block_device/Storage.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>

namespace mcs::block_device::storage
{
  [[nodiscard]] auto with_range
    ( Storage
    , core::memory::Range
    ) noexcept -> Storage;

  [[nodiscard]] auto with_range
    ( Storage
    , core::memory::Offset begin
    , core::memory::Offset end
    ) noexcept -> Storage;
}

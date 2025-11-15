// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/block_device/block/Size.hpp>

namespace mcs::block_device::meta_data::command
{
  struct BlockSize
  {
    constexpr BlockSize() noexcept = default;

    using Response = block::Size;
  };
}

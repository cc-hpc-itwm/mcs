// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <list>
#include <mcs/block_device/block/Range.hpp>

namespace mcs::block_device::meta_data::command
{
  struct Blocks
  {
    constexpr Blocks() noexcept = default;

    using Response = std::list<block::Range>;
  };
}

// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/block_device/block/Range.hpp>
#include <mcs/block_device/meta_data/Blocks.hpp>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::block_device::meta_data::command
{
  struct Remove
  {
    using Response = block_device::meta_data::Blocks::RemoveResult;

    block_device::block::Range range;
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<block_device::meta_data::command::Remove>
  {
    using Type = block_device::meta_data::command::Remove;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

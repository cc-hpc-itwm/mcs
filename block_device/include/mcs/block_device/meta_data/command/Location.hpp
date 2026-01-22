// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/block_device/block/ID.hpp>
#include <mcs/block_device/meta_data/Blocks.hpp>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::block_device::meta_data::command
{
  struct Location
  {
    using Response = block_device::meta_data::Blocks::Location;

    block_device::block::ID id;
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<block_device::meta_data::command::Location>
  {
    using Type = block_device::meta_data::command::Location;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

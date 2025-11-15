// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/block_device/meta_data/command/Add.hpp>
#include <mcs/block_device/meta_data/command/BlockSize.hpp>
#include <mcs/block_device/meta_data/command/Blocks.hpp>
#include <mcs/block_device/meta_data/command/Location.hpp>
#include <mcs/block_device/meta_data/command/NumberOfBlocks.hpp>
#include <mcs/block_device/meta_data/command/Remove.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::block_device::meta_data
{
  using Commands = util::type::List
    < command::BlockSize
    , command::NumberOfBlocks
    , command::Blocks
    , command::Add
    , command::Remove
    , command::Location
    >;
}

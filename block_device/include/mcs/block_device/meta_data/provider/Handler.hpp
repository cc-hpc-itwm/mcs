// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/block_device/meta_data/Blocks.hpp>
#include <mcs/block_device/meta_data/command/Add.hpp>
#include <mcs/block_device/meta_data/command/BlockSize.hpp>
#include <mcs/block_device/meta_data/command/Blocks.hpp>
#include <mcs/block_device/meta_data/command/Location.hpp>
#include <mcs/block_device/meta_data/command/NumberOfBlocks.hpp>
#include <mcs/block_device/meta_data/command/Remove.hpp>
#include <mcs/util/not_null.hpp>

namespace mcs::block_device::meta_data::provider
{
  struct Handler
  {
    Handler (util::not_null<Blocks>);

    auto operator() (command::BlockSize) const -> command::BlockSize::Response;
    auto operator()
      ( command::NumberOfBlocks
      ) const -> command::NumberOfBlocks::Response;
    auto operator() (command::Blocks) const -> command::Blocks::Response;
    auto operator() (command::Add) -> command::Add::Response;
    auto operator() (command::Remove) -> command::Remove::Response;
    auto operator() (command::Location) const -> command::Location::Response;

  private:
    util::not_null<Blocks> _blocks;
  };
}
